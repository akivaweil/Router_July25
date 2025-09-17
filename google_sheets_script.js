// Google Apps Script for Router Production Data Logging
// Copy this script into Google Apps Script (script.google.com)

function doPost(e) {
  try {
    // Parse the JSON data from ESP32
    const data = JSON.parse(e.postData.contents);
    
    // Get the active spreadsheet (create one if it doesn't exist)
    let sheet = getOrCreateSheet();
    
    // Add headers if this is the first row
    if (sheet.getLastRow() === 0) {
      sheet.getRange(1, 1, 1, 10).setValues([[
        'Timestamp', 'Total Cycles', 'Current Hour Cycles', 'Hour', 'Day', 'Month', 'Year',
        'Avg 1min', 'Avg 5min', 'Avg 15min', 'Avg 30min'
      ]]);
    }
    
    // Add the new data row
    sheet.appendRow([
      data.timestamp,
      data.total_cycles,
      data.current_hour_cycles,
      data.hour,
      data.day,
      data.month,
      data.year,
      data.avg_1min,
      data.avg_5min,
      data.avg_15min,
      data.avg_30min
    ]);
    
    // Return success response
    return ContentService
      .createTextOutput(JSON.stringify({status: 'success', message: 'Data logged successfully'}))
      .setMimeType(ContentService.MimeType.JSON);
      
  } catch (error) {
    // Return error response
    return ContentService
      .createTextOutput(JSON.stringify({status: 'error', message: error.toString()}))
      .setMimeType(ContentService.MimeType.JSON);
  }
}

function getOrCreateSheet() {
  // Try to get existing spreadsheet
  const files = DriveApp.getFilesByName('Router Production Data');
  
  if (files.hasNext()) {
    // Use existing spreadsheet
    const file = files.next();
    const spreadsheet = SpreadsheetApp.open(file);
    return spreadsheet.getActiveSheet();
  } else {
    // Create new spreadsheet
    const spreadsheet = SpreadsheetApp.create('Router Production Data');
    const sheet = spreadsheet.getActiveSheet();
    
    // Make the spreadsheet publicly viewable (optional)
    // spreadsheet.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
    
    return sheet;
  }
}

// Test function to verify the script works
function testScript() {
  const testData = {
    timestamp: '2024-01-01 12:00:00',
    total_cycles: 100,
    current_hour_cycles: 5,
    hour: 12,
    day: 1,
    month: 1,
    year: 2024,
    avg_1min: 2.5,
    avg_5min: 2.0,
    avg_15min: 1.8,
    avg_30min: 1.5
  };
  
  const mockEvent = {
    postData: {
      contents: JSON.stringify(testData)
    }
  };
  
  const result = doPost(mockEvent);
  console.log(result.getContent());
}
