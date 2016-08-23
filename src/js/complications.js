var util = require("./util");

var defaultGadgetDescription = "Select the gadget displayed in the center of the complication's ring.";

function gadgetSelect(gadgets, desc, group) {
  return {
    "type": "select",
    "label": "Gadget",
    "defaultValue": 0,
    "description": desc,
    "group": group,
    "options": util.selectOptions(gadgets)
  };
}

function noneComplication(position) {
  return [];
}

function batteryComplication(position) {
  return [
    gadgetSelect([
      "Icon",
      "Percent",
    ], defaultGadgetDescription, position + "-battery"),
  ];
}

function healthComplication(position) {
  return [
    gadgetSelect([
      "Status icon",
      "Distance walked",
      "KCal active",
      "KCal resting",
      "KCal total",
      "Time active",
    ], defaultGadgetDescription, position + "-health")
  ];
}

function dateComplication(position) {
  return [
    gadgetSelect([
      "Day of Week",
      "Month",
      "None",
    ],
    "Select the gadget displayed alongside the current date.",
    position + "-date")
  ];
}

function weatherComplication(position) {
  return [
    gadgetSelect([
      "Conditions (icon)",
      "Temperature (°F)",
      "Temperature (°C)",
      "Humidity (%)",
    ], defaultGadgetDescription, position + "-weather"),
  ];
}

function timezoneComplication(position) {
  return [
    {
      "type": "select",
      "label": "UTC Offset",
      "description": "Choose a UTC offset for the clock.  Note that due to ever-changing daylight savings time policies, this will not be adjusted for daylight savings time!",
      "defaultValue": 0,
      "group": position + "-timezone",
      "options": require("./timezones")
    }
  ];
}

var complicationTypes = [
  "none",
  "battery",
  "health",
  "date",
  "weather",
  "timezone"
];

exports.allConfigurators = function(position) {
  return [
    noneComplication(position),
    batteryComplication(position),
    healthComplication(position),
    dateComplication(position),
    weatherComplication(position),
    timezoneComplication(position),
  ];
}
