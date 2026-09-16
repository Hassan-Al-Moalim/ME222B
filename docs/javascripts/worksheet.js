/* ---------------------------------------------------------------------------
   ME 222B lab worksheets.

   Makes the blank cells in lab tables editable, saves what students type to
   this browser's localStorage, and offers the filled sheet as a Markdown
   download or a printable PDF.

   Storage is localStorage, not cookies: cookies are capped near 4 KB and get
   sent to the server on every request, neither of which we want for notes that
   never leave the machine.

   Nothing here is submitted anywhere. A student on a lab bench PC shares their
   entries with whoever uses that browser next.
   --------------------------------------------------------------------------- */

(function () {
  "use strict";

  var BLANK = /^_{3,}$/;                 // the `______` placeholder in the markdown
  var NS = "me222b";

  function pageKey() {
    return NS + ":" + window.location.pathname;
  }

  /* --- what can be typed into --------------------------------------------
     Two shapes: table cells holding a blank ("____ cm"), and inline `______`
     blanks in prose, as used by the problem statement. Code blocks are
     excluded — the starter sketch uses `___` as its own placeholder and must
     stay read-only.
     ----------------------------------------------------------------------- */

  function collectFillables(container) {
    var sheet = container.getAttribute("data-worksheet") || "sheet";
    var out = [];

    Array.prototype.forEach.call(container.querySelectorAll("tbody tr"), function (tr, r) {
      Array.prototype.forEach.call(tr.cells, function (td, c) {
        if (/_{3,}/.test((td.textContent || "").trim())) {
          out.push({ el: td, sheet: sheet, key: "r" + r + "c" + c });
        }
      });
    });

    var n = 0;
    Array.prototype.forEach.call(container.querySelectorAll("code"), function (code) {
      if (code.closest("pre")) return;                 // never touch sample code
      if (code.closest("td, th")) return;              // already handled above
      if (!BLANK.test((code.textContent || "").trim())) return;
      out.push({ el: code, sheet: sheet, key: "i" + (n++) });
    });

    return out;
  }

  /* --- make them editable and restore saved values ----------------------- */

  function activate(container) {
    var fillables = collectFillables(container);
    if (!fillables.length) return false;

    fillables.forEach(function (cell) {
      var td = cell.el;
      var key = pageKey() + ":" + cell.sheet + ":" + cell.key;

      // Keep any unit suffix ("____ cm") as a visible hint next to the input.
      var raw = (td.textContent || "").trim();
      var suffix = raw.replace(/_{3,}/, "").trim();
      var blank = (raw.match(/_{3,}/) || ["______"])[0];

      td.textContent = "";
      td.classList.add("ws-cell");

      var input = document.createElement("span");
      input.className = "ws-input";
      input.setAttribute("contenteditable", "true");
      input.setAttribute("role", "textbox");
      input.setAttribute("aria-label", rowLabel(td) + " value");
      input.setAttribute("spellcheck", "false");

      // A long run of underscores means a long answer — size the field to match
      // so the layout hints at how much is expected.
      if (blank.length > 12) {
        input.classList.add("ws-input--wide");
        input.setAttribute("spellcheck", "true");
        input.setAttribute("data-hint", "type your answer");
      } else {
        // The empty-state hint is drawn by CSS from this attribute, so it must
        // always be set or the field reads as blank rather than fillable.
        input.setAttribute("data-hint", suffix ? "0" : "GPIO");
      }

      var saved = null;
      try { saved = window.localStorage.getItem(key); } catch (e) { /* private mode */ }
      if (saved) input.textContent = saved;

      input.addEventListener("input", function () {
        try { window.localStorage.setItem(key, input.textContent.trim()); }
        catch (e) { /* quota or private mode — typing still works this session */ }
        flashSaved();
      });

      // Enter should move on, not insert a newline into a one-line field.
      input.addEventListener("keydown", function (ev) {
        if (ev.key === "Enter") { ev.preventDefault(); focusNext(input); }
      });

      td.appendChild(input);
      if (suffix) {
        var unit = document.createElement("span");
        unit.className = "ws-unit";
        unit.textContent = suffix;
        td.appendChild(unit);
      }
    });

    return true;
  }

  function rowLabel(td) {
    var tr = td.closest("tr");
    if (tr && tr.cells[0]) return (tr.cells[0].textContent || "").trim();
    return "answer";
  }

  function focusNext(current) {
    var all = Array.prototype.slice.call(document.querySelectorAll(".ws-input"));
    var i = all.indexOf(current);
    if (i > -1 && i + 1 < all.length) all[i + 1].focus();
    else current.blur();
  }

  /* --- "saved" indicator -------------------------------------------------- */

  var flashTimer = null;
  function flashSaved() {
    var el = document.querySelector(".ws-status");
    if (!el) return;
    el.textContent = "Saved";
    el.classList.add("is-on");
    clearTimeout(flashTimer);
    flashTimer = setTimeout(function () { el.classList.remove("is-on"); }, 1400);
  }

  /* --- export ------------------------------------------------------------- */

  function currentTitle() {
    var h1 = document.querySelector("article h1");
    return h1 ? h1.textContent.replace(/¶$/, "").trim() : document.title;
  }

  function toMarkdown() {
    var lines = ["# " + currentTitle(), ""];
    lines.push("Worksheet exported " + new Date().toLocaleString() + ".");
    lines.push("");

    document.querySelectorAll("[data-worksheet]").forEach(function (container) {
      var heading = container.querySelector("strong, p");
      if (heading) lines.push("## " + heading.textContent.trim(), "");

      var prose = container.querySelectorAll("blockquote");
      if (prose.length) {
        prose.forEach(function (bq) {
          bq.querySelectorAll("p").forEach(function (para) {
            var t = (para.textContent || "").trim().replace(/\s+/g, " ");
            if (t) lines.push("> " + t, ">");
          });
          lines.push("");
        });
      }

      container.querySelectorAll("table").forEach(function (table) {
        var head = table.querySelectorAll("thead th");
        if (head.length) {
          lines.push("| " + Array.prototype.map.call(head, function (th) {
            return th.textContent.trim();
          }).join(" | ") + " |");
          lines.push("| " + Array.prototype.map.call(head, function () {
            return "---";
          }).join(" | ") + " |");
        }
        table.querySelectorAll("tbody tr").forEach(function (tr) {
          lines.push("| " + Array.prototype.map.call(tr.cells, cellText).join(" | ") + " |");
        });
        lines.push("");
      });
    });

    return lines.join("\n");
  }

  // A filled cell exports as "4.0 m"; an empty one as an em dash, never as a
  // bare unit. Static cells export their text unchanged.
  function cellText(td) {
    var input = td.querySelector(".ws-input");
    if (!input) return (td.textContent || "").trim().replace(/\s+/g, " ") || "—";

    var value = (input.textContent || "").trim();
    if (!value) return "—";

    var unitEl = td.querySelector(".ws-unit");
    var unit = unitEl ? (unitEl.textContent || "").trim() : "";
    return unit ? value + " " + unit : value;
  }

  function download(text, filename) {
    var blob = new Blob([text], { type: "text/markdown;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function () { URL.revokeObjectURL(url); }, 1000);
  }

  function slug() {
    var parts = window.location.pathname.split("/").filter(Boolean);
    return (parts[parts.length - 1] || "worksheet").replace(/[^a-z0-9-]/gi, "") + "-answers";
  }

  function clearAll() {
    if (!window.confirm("Clear everything you have typed on this page? This cannot be undone.")) return;
    var prefix = pageKey() + ":";
    try {
      Object.keys(window.localStorage)
        .filter(function (k) { return k.indexOf(prefix) === 0; })
        .forEach(function (k) { window.localStorage.removeItem(k); });
    } catch (e) { /* ignore */ }
    document.querySelectorAll(".ws-input").forEach(function (el) { el.textContent = ""; });
  }

  /* --- toolbar ------------------------------------------------------------ */

  function toolbar() {
    var bar = document.createElement("div");
    bar.className = "ws-toolbar";
    bar.innerHTML =
      '<span class="ws-note">Fill these tables in as you work — your entries are saved in this browser.</span>' +
      '<span class="ws-actions">' +
        '<button type="button" class="ws-btn" data-act="md">Download answers</button>' +
        '<button type="button" class="ws-btn ws-btn--quiet" data-act="clear">Clear</button>' +
        '<span class="ws-status" aria-live="polite"></span>' +
      '</span>';

    bar.addEventListener("click", function (ev) {
      var btn = ev.target.closest(".ws-btn");
      if (!btn) return;
      var act = btn.getAttribute("data-act");
      if (act === "md") download(toMarkdown(), slug() + ".md");
      else if (act === "clear") clearAll();
    });

    return bar;
  }

  /* --- boot --------------------------------------------------------------- */

  function init() {
    var containers = document.querySelectorAll("[data-worksheet]");
    if (!containers.length) return;

    var any = false;
    Array.prototype.forEach.call(containers, function (c) {
      if (activate(c)) any = true;
    });
    if (!any) return;

    // The page-actions script wraps the h1 in a flex row. Insert after that
    // wrapper, not after the h1, or the toolbar lands inside the row and
    // pushes the Copy page button onto its own line.
    var article = document.querySelector("article");
    if (!article) return;
    var h1 = article.querySelector("h1");
    if (!h1) return;
    var anchor = h1.closest(".page-actions-row") || h1;
    anchor.parentNode.insertBefore(toolbar(), anchor.nextSibling);
  }

  if (typeof document$ !== "undefined" && document$.subscribe) {
    document$.subscribe(init);          // survives Material's page swaps
  } else if (document.readyState !== "loading") {
    init();
  } else {
    document.addEventListener("DOMContentLoaded", init);
  }
})();
