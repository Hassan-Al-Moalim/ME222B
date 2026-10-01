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

  // A blank is a run of two or more underscores, alone in the cell or followed
  // by a unit ("____ cm"). Two is the floor because authors write whatever
  // length looks right in the source, and a stricter rule silently leaves
  // fields unfillable.
  var BLANK = /^_{2,}$/;
  var CELL_BLANK = /^_{2,}(\s+\S+)?$/;
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
        var text = (td.textContent || "").trim();

        // A blank marker, or an empty cell outside the label column. The empty
        // case matters because an author writing a Notes column naturally
        // leaves it blank rather than typing underscores into it.
        var fillable = CELL_BLANK.test(text) || (text === "" && c > 0);

        // An averaged column is derived from the run columns, so it is filled
        // by the page rather than typed into.
        if (fillable && !isComputedCell(container, td)) {
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
      var blank = (raw.match(/_{2,}/) || ["______"])[0];

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
      var head = columnHeading(td);
      if (/note|comment|observation|remark/.test(head)) {
        input.classList.add("ws-input--wide");
        input.setAttribute("spellcheck", "true");
        input.setAttribute("data-hint", "notes");
      } else if (blank.length > 12) {
        input.classList.add("ws-input--wide");
        input.setAttribute("spellcheck", "true");
        input.setAttribute("data-hint", "type your answer");
      } else {
        // The empty-state hint is drawn by CSS from this attribute, so it must
        // always be set or the field reads as blank rather than fillable.
        input.setAttribute("data-hint", hintFor(td, suffix));
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

  // The hint has to come from the column, not a fixed string: "GPIO" belongs
  // over a pin table and is nonsense over a column of encoder counts.
  function isComputedCell(container, td) {
    return container.hasAttribute("data-runs-max") && /\(avg\)/i.test(columnHeading(td));
  }

  function columnHeading(td) {
    var table = td.closest ? td.closest("table") : null;
    if (!table || typeof td.cellIndex !== "number") return "";
    var headRow = table.querySelector("thead tr");
    var th = headRow && headRow.cells[td.cellIndex];
    return th ? (th.textContent || "").toLowerCase() : "";
  }

  function hintFor(td, suffix) {
    if (suffix) return "0";                       // a unit follows the field

    var head = columnHeading(td);

    if (/\bpins?\b|gpio/.test(head)) return "GPIO";
    if (/count|rpm|speed|cpr|pwm|duty|dist|drift|time|angle|error|volt|current|hyster|avg|max|min|ratio|gain|freq|%|\(m\)|\bms\b|\bcm\b|\bs\b/.test(head)) return "0";
    return "—";
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

  /* --- repeated runs and their average ------------------------------------
     Every run column exists in the markup from the start; the extra ones are
     hidden rather than inserted on demand. Inserting a column would shift the
     cell indices that saved values are keyed by, silently moving every entry
     to the right of it.
     ----------------------------------------------------------------------- */

  function columnIndexes(table, pattern) {
    var headRow = table.querySelector("thead tr");
    var out = [];
    if (!headRow) return out;
    Array.prototype.forEach.call(headRow.cells, function (th, i) {
      if (pattern.test((th.textContent || "").trim())) out.push(i);
    });
    return out;
  }

  function setColumnVisible(table, index, visible) {
    var headRow = table.querySelector("thead tr");
    if (headRow && headRow.cells[index]) {
      headRow.cells[index].style.display = visible ? "" : "none";
    }
    table.querySelectorAll("tbody tr").forEach(function (tr) {
      if (tr.cells[index]) tr.cells[index].style.display = visible ? "" : "none";
    });
  }

  function setupRuns(container) {
    if (!container.hasAttribute("data-runs-max")) return;

    var table = container.querySelector("table");
    if (!table) return;

    var sheet = container.getAttribute("data-worksheet") || "sheet";
    var runCols = columnIndexes(table, /^run\s*\d+/i);
    var avgCols = columnIndexes(table, /\(avg\)/i);
    if (!runCols.length || !avgCols.length) return;

    var avgCol = avgCols[0];
    var divisor = parseFloat(container.getAttribute("data-avg-divisor")) || 1;
    var max = Math.min(parseInt(container.getAttribute("data-runs-max"), 10) || runCols.length,
                       runCols.length);
    var start = parseInt(container.getAttribute("data-runs"), 10) || 2;
    var countKey = pageKey() + ":" + sheet + ":runs";

    var shown = start;
    try {
      var stored = parseInt(window.localStorage.getItem(countKey), 10);
      if (stored >= start && stored <= max) shown = stored;
    } catch (e) { /* private mode */ }

    // Mark the average column as derived, not typed.
    table.querySelectorAll("tbody tr").forEach(function (tr) {
      var td = tr.cells[avgCol];
      if (!td) return;
      td.classList.add("ws-computed");
      td.textContent = "—";
    });

    function recompute() {
      table.querySelectorAll("tbody tr").forEach(function (tr) {
        var sum = 0, n = 0;
        for (var i = 0; i < shown; i++) {
          var cell = tr.cells[runCols[i]];
          var input = cell && cell.querySelector(".ws-input");
          var value = input ? parseFloat((input.textContent || "").trim()) : NaN;
          if (!isNaN(value)) { sum += value; n += 1; }
        }
        var out = tr.cells[avgCol];
        if (!out) return;
        if (!n) { out.textContent = "—"; out.removeAttribute("title"); return; }
        var avg = (sum / n) / divisor;
        out.textContent = (Math.round(avg * 10) / 10).toString();
        out.title = "mean of " + n + " run" + (n === 1 ? "" : "s") +
                    (divisor !== 1 ? ", divided by " + divisor : "");
      });
    }

    function apply() {
      runCols.forEach(function (index, i) { setColumnVisible(table, index, i < shown); });
      if (button) {
        button.disabled = shown >= max;
        button.textContent = shown >= max ? "All " + max + " runs shown" : "Add a run";
      }
      recompute();
    }

    var button = document.createElement("button");
    button.type = "button";
    button.className = "ws-btn ws-btn--quiet ws-addrun";
    button.addEventListener("click", function () {
      if (shown >= max) return;
      shown += 1;
      try { window.localStorage.setItem(countKey, String(shown)); } catch (e) { /* ignore */ }
      apply();
    });
    container.appendChild(button);

    container.addEventListener("input", recompute);
    apply();
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
      setupRuns(c);
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

  /* --- shared with the importer -------------------------------------------
     The collector page reads the same localStorage rows, so it has to use the
     same key scheme and the same idea of what counts as a blank. Exporting the
     real functions keeps one implementation rather than two that drift. */

  window.ME222B = window.ME222B || {};
  window.ME222B.worksheet = {
    namespace: NS,
    collect: collectFillables,
    keyFor: function (pagePath, sheet, id) {
      return NS + ":" + pagePath + ":" + sheet + ":" + id;
    }
  };
})();
