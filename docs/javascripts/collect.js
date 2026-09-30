/* ---------------------------------------------------------------------------
   Collect everything a student has typed into the lab worksheets and show it
   on one page, ready to paste into a report.

   Nothing is generated or interpreted here: it fetches each lab page, finds the
   same worksheet blocks, and fills them with the values already saved in this
   browser. Empty fields stay empty.
   --------------------------------------------------------------------------- */

(function () {
  "use strict";

  function labLinks() {
    var seen = {};
    var out = [];
    document.querySelectorAll('.md-nav a[href]').forEach(function (a) {
      var url = new URL(a.getAttribute("href"), window.location.href);
      if (!/\/labs\/lab\d+/.test(url.pathname)) return;
      if (seen[url.pathname]) return;
      seen[url.pathname] = true;
      out.push({ path: url.pathname, title: a.textContent.trim() });
    });
    return out.sort(function (a, b) {
      var na = parseInt((a.path.match(/lab(\d+)/) || [])[1] || 0, 10);
      var nb = parseInt((b.path.match(/lab(\d+)/) || [])[1] || 0, 10);
      return na - nb;
    });
  }

  function saved(path, sheet, id) {
    try {
      return window.localStorage.getItem(window.ME222B.worksheet.keyFor(path, sheet, id));
    } catch (e) {
      return null;
    }
  }

  // Replace each blank in a fetched page with what the student typed, and
  // report how many were filled so empty labs can be skipped.
  function fill(container, path) {
    var items = window.ME222B.worksheet.collect(container);
    var filled = 0;

    items.forEach(function (item) {
      var value = saved(path, item.sheet, item.key);
      var raw = (item.el.textContent || "").trim();
      var unit = raw.replace(/_{2,}/, "").trim();

      if (value) {
        item.el.textContent = unit ? value + " " + unit : value;
        item.el.classList.add("rp-filled");
        filled += 1;
      } else {
        item.el.textContent = "—";
        item.el.classList.add("rp-empty");
      }
    });

    return filled;
  }

  function strip(node) {
    // Drop anything that only makes sense on the live lab page.
    node.querySelectorAll("a").forEach(function (a) {
      a.replaceWith(document.createTextNode(a.textContent));
    });
    node.querySelectorAll("script, .headerlink").forEach(function (n) { n.remove(); });
    return node;
  }

  function heading(text, level) {
    var h = document.createElement(level || "h2");
    h.textContent = text;
    return h;
  }

  function render(root, results) {
    root.innerHTML = "";

    var any = results.some(function (r) { return r.filled > 0; });
    if (!any) {
      var p = document.createElement("p");
      p.className = "rp-none";
      p.textContent =
        "Nothing saved yet in this browser. Fill in the tables on a lab page, " +
        "then come back — entries are stored per browser, so use the same one.";
      root.appendChild(p);
      return;
    }

    results.forEach(function (r) {
      if (!r.filled) return;

      root.appendChild(heading(r.title));

      var meta = document.createElement("p");
      meta.className = "rp-meta";
      meta.textContent = r.filled + " of " + r.total + " fields filled";
      root.appendChild(meta);

      r.blocks.forEach(function (block) { root.appendChild(block); });
    });
  }

  function toMarkdown(root) {
    var lines = ["# ME 222B — my lab data", "",
                 "Collected " + new Date().toLocaleString() + ".", ""];

    root.querySelectorAll("h2, p, table, blockquote").forEach(function (node) {
      if (node.tagName === "H2") {
        lines.push("", "## " + node.textContent.trim(), "");
        return;
      }

      // The label above a table ("**Left driver**") is the only thing telling
      // two identical-looking tables apart, so it has to survive the export.
      if (node.tagName === "P") {
        var strong = node.querySelector("strong");
        if (strong && node.textContent.trim() === strong.textContent.trim() + (node.textContent.trim().slice(strong.textContent.trim().length) || "")) {
          var label = node.textContent.trim().replace(/\s+/g, " ");
          if (label && !node.classList.contains("rp-meta")) lines.push("**" + label + "**", "");
        }
        return;
      }

      if (node.tagName === "BLOCKQUOTE") {
        node.querySelectorAll("p").forEach(function (p) {
          var t = p.textContent.trim().replace(/\s+/g, " ");
          if (t) lines.push("> " + t, ">");
        });
        lines.push("");
        return;
      }

      var head = node.querySelectorAll("thead th");
      if (head.length) {
        lines.push("| " + Array.prototype.map.call(head, function (th) {
          return th.textContent.trim();
        }).join(" | ") + " |");
        lines.push("| " + Array.prototype.map.call(head, function () { return "---"; }).join(" | ") + " |");
      }
      node.querySelectorAll("tbody tr").forEach(function (tr) {
        lines.push("| " + Array.prototype.map.call(tr.cells, function (td) {
          return (td.textContent || "").trim().replace(/\s+/g, " ") || "—";
        }).join(" | ") + " |");
      });
      lines.push("");
    });

    return lines.join("\n");
  }

  function download(text) {
    var blob = new Blob([text], { type: "text/markdown;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.href = url;
    a.download = "me222b-lab-data.md";
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function () { URL.revokeObjectURL(url); }, 1000);
  }

  function init() {
    var root = document.getElementById("collected");
    if (!root || !window.ME222B || !window.ME222B.worksheet) return;

    root.textContent = "Looking through your saved work…";

    var labs = labLinks();
    Promise.all(labs.map(function (lab) {
      return fetch(lab.path, { cache: "no-cache" })
        .then(function (r) { return r.ok ? r.text() : ""; })
        .then(function (html) {
          if (!html) return { title: lab.title, filled: 0, total: 0, blocks: [] };

          var doc = new DOMParser().parseFromString(html, "text/html");
          var blocks = [];
          var filled = 0;
          var total = 0;

          doc.querySelectorAll("[data-worksheet]").forEach(function (w) {
            total += window.ME222B.worksheet.collect(w).length;
            filled += fill(w, lab.path);
            blocks.push(strip(document.importNode(w, true)));
          });

          return { title: lab.title, filled: filled, total: total, blocks: blocks };
        })
        .catch(function () {
          return { title: lab.title, filled: 0, total: 0, blocks: [] };
        });
    })).then(function (results) {
      render(root, results);

      var bar = document.querySelector(".rp-actions");
      if (bar) {
        bar.hidden = false;
        bar.addEventListener("click", function (ev) {
          var btn = ev.target.closest("button");
          if (!btn) return;
          if (btn.dataset.act === "md") download(toMarkdown(root));
          if (btn.dataset.act === "print") window.print();
        });
      }
    });
  }

  if (typeof document$ !== "undefined" && document$.subscribe) {
    document$.subscribe(init);
  } else if (document.readyState !== "loading") {
    init();
  } else {
    document.addEventListener("DOMContentLoaded", init);
  }
})();
