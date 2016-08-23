exports.section = function(title, items) {
  items.unshift({
    "type": "heading",
    "defaultValue": title
  });
  return {
    "type": "section",
    "items": items
  };
}

exports.selectOption = function(label, value) {
  return {
    "label": label,
    "value": value
  };
}

exports.selectOptions = function(labels) {
  var idx = 0;
  return labels.map(function (l) { return exports.selectOption(l, idx++); });
}
