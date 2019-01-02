From: https://github.com/electronicsguy/ESP8266/tree/master/HTTPSRedirect

The key steps for creating your own copy of the Spreadsheet and activating the script:

Create a new Google Spreadsheet in your account.

Goto Tools->Script editor... and copy all the code from GoogleScript.gs into the editor.

You can goto the menu item "ESP8266 Logging -> Clear" to clear all the row contents from the sheet.

Get your spreadsheet-id. If you look at the URL in the browser tab where the spreadsheet is open, it'll be of the form: https://docs.google.com/spreadsheets/d/<random-string>/edit#gid=0. The <random string> is your spreadsheet-id. Put this value in GoogleScript.gs in the relevant line.
  
Publish your script by deploying it as a web app. The permissions should be set to "Execute the app as: Me (your email)" and "who has access to the app: Anyone, even anonymous". Once you publish your script, the script editor popup will say: "This project is now deployed as a web app.". It'll also display the URL for the webapp. Grab the string of random characters between "/macros/s/" and /"exec".

Example: If your URL looks like this: https://script.google.com/macros/s/XXXYYY/exec, then "XXXYYY" is your GScriptId. Put this value in GoogleDocs.ino for it to hit your script instead of mine.

Important Note: You need to re-publish your web-app (with a new version number) every time any change any made to your script. Google Apps script serves requests only with a published version of your script, not necessarily the latest one. This is an unfortunate Google Apps Script limitation. However, your GScriptId will remain the same.

Once these steps are completed, re-flash your ESP82666 with the new spreadsheet-id code. It should then read and write to your copy of the spreadsheet.
