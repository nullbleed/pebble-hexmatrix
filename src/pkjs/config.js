module.exports = [
  {
    "type": "heading",
    "defaultValue": "Hexmatrix Configuration"
  },
  {
    "type": "text",
    "defaultValue": "This is the settings panel for hexmatrix."
  },
  {
    "type": "text",
    "defaultValue": "Pay attention to the color selection on monochrome displays."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0x000000",
        "label": "Background Color"
      },
      {
        "type": "color",
        "messageKey": "AccentColor",
        "defaultValue": "0xFFFFFF",
        "label": "Accent Color"
      },
      {
        "type": "color",
        "messageKey": "MainColor",
        "defaultValue": "0x0000FF",
        "label": "Time Color"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
