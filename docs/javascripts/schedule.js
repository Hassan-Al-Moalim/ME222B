/* Colour-code the schedule rows by session type.

   The table stays plain Markdown so it is easy to edit; the row class is
   derived from the first cell rather than hand-written into HTML. */

(function () {
  "use strict";

  function classify() {
    var wrap = document.querySelector(".schedule");
    if (!wrap) return;

    wrap.querySelectorAll("tbody tr").forEach(function (tr) {
      var first = (tr.cells[0] ? tr.cells[0].textContent : "").trim();
      var all = tr.textContent;

      if (/^LAB\b/i.test(first)) tr.classList.add("is-lab");
      else if (/^(QUIZ|EXAM)\b/i.test(first)) tr.classList.add("is-assessment");
      else if (/National Day|Semester break/i.test(all)) tr.classList.add("is-off");
    });
  }

  if (typeof document$ !== "undefined" && document$.subscribe) {
    document$.subscribe(classify);
  } else if (document.readyState !== "loading") {
    classify();
  } else {
    document.addEventListener("DOMContentLoaded", classify);
  }
})();
