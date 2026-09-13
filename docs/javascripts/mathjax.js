window.MathJax = {
  tex: {
    inlineMath: [["\\(", "\\)"], ["$", "$"]],
    displayMath: [["\\[", "\\]"], ["$$", "$$"]],
    processEscapes: true,
    processEnvironments: true
  },
  options: { ignoreHtmlClass: ".*|", processHtmlClass: "arithmatex" }
};

// Re-typeset on Material's instant-navigation page swaps.
//
// Guarded deliberately: MathJax loads from a CDN, so on a slow or filtered
// network `MathJax.startup` may not exist yet (or ever). Throwing here would
// terminate Material's shared `document$` observable and take every later
// subscriber down with it — copy buttons, linked content tabs, and more would
// silently stop working. Failing quietly costs only unrendered math.
document$.subscribe(function () {
  if (typeof MathJax === "undefined" || !MathJax.startup || !MathJax.typesetPromise) {
    return;
  }
  try {
    MathJax.startup.output.clearCache();
    MathJax.typesetClear();
    MathJax.texReset();
    MathJax.typesetPromise();
  } catch (e) {
    console.warn("MathJax typeset skipped:", e);
  }
});
