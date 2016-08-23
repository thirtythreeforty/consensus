var complications = require("./complications");
var util = require("./util");

function complication(position) {
  var complication = util.section(position + " Complication", [
    {
      "type": "select",
      "label": "Type",
      "defaultValue": 0,
      "id": position + "-type",
      "messageKey": "KEY_PREF_" + position.toUpperCase() + "_COMPLICATION",
      "options": util.selectOptions([
        "None",
        "Battery",
        "Health",
        "Date",
        "Weather",
        "Time Zone",
      ])
    },
    complications.allConfigurators(position)
  ]);

  return complication;
}

function buildClayConfig() {
  var complications = ["Left", "Right", "Bottom"]
    .map(function(p) { return complication(p) });

  var config = [
    {
      "type": "heading",
      "defaultValue": "<center>Consensus Configuration</center>"
    },
    util.section("Appearance", [
      {
        "type": "select",
        "label": "Theme",
        "messageKey": "KEY_PREF_THEME",
        "defaultValue": 0,
        "options": util.selectOptions([
          "Bold Dark",
          "Bold White",
          "Thin Dark",
          "Minimal Dark",
          "Minimal White",
        ])
      },
      {
        "type": "color",
        "label": "Hand accent color",
        "messageKey": "KEY_PREF_HAND_COLOR",
        "sunlight": false,
        "layout": [
          [false, "00aaff"],
          ["0055ff", "0000ff"],
        ],
        "capabilities": ["COLOR"]
      },
      {
        "type": "toggle",
        "label": "Second hand",
        "messageKey": "KEY_PREF_SHOW_SECOND_HAND",
        "description": "Display the second hand.  Note that this may increase battery consumption.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Animations",
        "messageKey": "KEY_PREF_SHOW_SECOND_ANIMATIONS",
        "description": "Animate various complication updates and spin the hands when the watchface starts.",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": '"No Connection" icon',
        "messageKey": "KEY_PREF_SHOW_NO_CONNECTION",
        "description": 'Display a "no connection" icon when the watch is disconnected from the phone.',
        "defaultValue": true
      },
    ]),
    util.section("Behavior", [
      {
        "type": "toggle",
        "label": "Vibrate on the hour",
        "messageKey": "KEY_PREF_VIBRATE_ON_HOUR",
        "description": "Briefly vibrate once at the top of every hour, and twice at noon and midnight.",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Vibrate on disconnect",
        "messageKey": "KEY_PREF_VIBRATE_ON_DISCONNECT",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Vibrate on connect",
        "messageKey": "KEY_PREF_VIBRATE_ON_CONNECT",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Quiet while sleeping",
        "messageKey": "KEY_PREF_QUIET_DURING_SLEEP",
        "description": "When you are sleeping, disable all these vibrations (but don't worry: alarms and timers will still buzz). Has no effect if Pebble Health is disabled.",
        "defaultValue": false
      },
      {
        "type": "select",
        "label": "Location source",
        // TODO send this back custom-like
        "description": 'Set the location to use when fetching weather data. If "Current," your phone\'s location services must be enabled.',
        "defaultValue": 0,
        "options": util.selectOptions([
          "Current",
          "Manual",
        ])
      }
    ]),
    complications,
    util.section("Miscellaneous", [
      {
        "type": "text",
        "defaultValue": require("./donation")
      },
    ]),
    {
      "type": "submit",
      "defaultValue": "Save"
    }
  ];
  return config;
}

exports.config = buildClayConfig();
