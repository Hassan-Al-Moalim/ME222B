/* ---------------------------------------------------------------------------
   Page actions: copy, view, or download this page's Markdown source.

   The source is published next to each page by hooks/publish_markdown.py, so
   what you get is the real file rather than Markdown reconstructed from the
   rendered DOM.
   --------------------------------------------------------------------------- */

(function () {
  "use strict";

  function sourceUrl() {
    var path = window.location.pathname;
    return path.endsWith("/") ? path + "index.md" : path + "/index.md";
  }

  function filename() {
    var parts = window.location.pathname.split("/").filter(Boolean);
    var last = parts[parts.length - 1] || "index";
    return last.replace(/[^a-z0-9._-]/gi, "") + ".md";
  }

  function fetchSource() {
    return fetch(sourceUrl(), { cache: "no-cache" }).then(function (r) {
      if (!r.ok) throw new Error("source not published (" + r.status + ")");
      return r.text();
    });
  }

  function flash(button, text) {
    var original = button.getAttribute("data-label") || button.textContent;
    button.setAttribute("data-label", original);
    button.textContent = text;
    setTimeout(function () { button.textContent = original; }, 1600);
  }

  // Writing to the clipboard can be refused even on a secure origin — most
  // often with "Document is not focused" — so the textarea fallback has to
  // cover a rejected promise, not just a missing API.
  function legacyCopy(text) {
    return new Promise(function (resolve, reject) {
      var ta = document.createElement("textarea");
      ta.value = text;
      ta.setAttribute("readonly", "");
      ta.style.position = "fixed";
      ta.style.top = "-1000px";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      ta.setSelectionRange(0, text.length);
      var ok = false;
      try { ok = document.execCommand("copy"); } catch (e) { ok = false; }
      document.body.removeChild(ta);
      ok ? resolve() : reject(new Error("copy command refused"));
    });
  }

  function copy(text) {
    if (navigator.clipboard && window.isSecureContext) {
      return navigator.clipboard.writeText(text).catch(function () {
        return legacyCopy(text);
      });
    }
    return legacyCopy(text);
  }

  function download(text) {
    var blob = new Blob([text], { type: "text/markdown;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.href = url;
    a.download = filename();
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function () { URL.revokeObjectURL(url); }, 1000);
  }

  function build() {
    var wrap = document.createElement("div");
    wrap.className = "page-actions";
    wrap.innerHTML =
      '<button type="button" class="pa-btn pa-btn--main" data-act="copy">' +
        '<svg viewBox="0 0 24 24" width="14" height="14" aria-hidden="true">' +
        '<path fill="currentColor" d="M16 1H4a2 2 0 0 0-2 2v14h2V3h12V1Zm3 4H8a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h11a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2Zm0 16H8V7h11v14Z"/></svg>' +
        '<span>Copy page</span>' +
      '</button>' +
      '<button type="button" class="pa-btn pa-btn--toggle" aria-haspopup="true" aria-expanded="false" aria-label="More page actions">' +
        '<svg viewBox="0 0 24 24" width="14" height="14" aria-hidden="true"><path fill="currentColor" d="M7 10l5 5 5-5z"/></svg>' +
      '</button>' +
      '<div class="pa-menu" hidden>' +
        '<button type="button" class="pa-item" data-act="copy">' +
          '<strong>Copy page</strong><span>Copy this page as Markdown</span></button>' +
        '<a class="pa-item" data-act="view" target="_blank" rel="noopener">' +
          '<strong>View as Markdown</strong><span>Open this page as plain text</span></a>' +
        '<button type="button" class="pa-item" data-act="download">' +
          '<strong>Download Markdown</strong><span>Save the .md file</span></button>' +
      '</div>';

    var menu = wrap.querySelector(".pa-menu");
    var toggle = wrap.querySelector(".pa-btn--toggle");
    wrap.querySelector('[data-act="view"]').href = sourceUrl();

    function close() {
      menu.hidden = true;
      toggle.setAttribute("aria-expanded", "false");
    }

    toggle.addEventListener("click", function (ev) {
      ev.stopPropagation();
      var open = menu.hidden;
      menu.hidden = !open;
      toggle.setAttribute("aria-expanded", String(open));
    });

    document.addEventListener("click", function (ev) {
      if (!wrap.contains(ev.target)) close();
    });

    document.addEventListener("keydown", function (ev) {
      if (ev.key === "Escape") close();
    });

    wrap.addEventListener("click", function (ev) {
      var el = ev.target.closest("[data-act]");
      if (!el) return;
      var act = el.getAttribute("data-act");
      if (act === "view") { close(); return; }        // plain link

      ev.preventDefault();
      var label = el.classList.contains("pa-item") ? el.querySelector("strong") : el.querySelector("span");

      fetchSource().then(function (text) {
        if (act === "copy") {
          return copy(text).then(function () { flash(label, "Copied"); });
        }
        download(text);
        flash(label, "Downloaded");
      }).catch(function (err) {
        flash(label, "Unavailable");
        console.warn("page actions:", err);
      }).then(close);
    });

    return wrap;
  }

  function init() {
    if (document.querySelector(".page-actions")) return;
    var article = document.querySelector("article.md-content__inner, article");
    if (!article) return;
    var h1 = article.querySelector("h1");
    if (!h1) return;

    var row = document.createElement("div");
    row.className = "page-actions-row";
    h1.parentNode.insertBefore(row, h1);
    row.appendChild(h1);
    row.appendChild(build());
  }

  if (typeof document$ !== "undefined" && document$.subscribe) {
    document$.subscribe(init);
  } else if (document.readyState !== "loading") {
    init();
  } else {
    document.addEventListener("DOMContentLoaded", init);
  }
})();
