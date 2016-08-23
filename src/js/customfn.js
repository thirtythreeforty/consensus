module.exports = function(minified) {
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;

  // These must be replicated here because the function is .toString()'d
  var complicationTypes = [
    "none",
    "battery",
    "health",
    "date",
    "weather",
    "timezone",
  ];
  var positions = ["Left", "Bottom", "Right"];

  // https://github.com/pebble/clay/pull/130
  clayConfig.getItemsByGroup = function(group) {
    return clayConfig.getAllItems().filter(function(item) {
      return item.config.group === group;
    });
  };

  function hideAll(pos) {
    for(var n = 0; n < complicationTypes.length; ++n) {
      var cType = complicationTypes[n];
      var items = clayConfig.getItemsByGroup(pos + "-" + cType);
      for(var t = 0; t < items.length; ++t) {
        items[t].hide();
      }
    }
  }

  function showGroup(pos, group) {
    var configurators = clayConfig.getItemsByGroup(pos + "-" + group);
    for(var i = 0; i < configurators.length; ++i) {
      configurators[i].show();
      // TODO load the configuration if required
    }
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    // Hide all configurators, and bind the type choosers
    for(var i = 0; i < positions.length; ++i) { (function() {
      var pos = positions[i];
      hideAll(pos);

      clayConfig.getItemById(pos + "-type").on("change", function() {
        hideAll(pos);
        showGroup(pos, complicationTypes[this.get()]);
      });
    })(); }
  });
}
