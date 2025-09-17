# Google Sheets Cloud Storage Setup

## Super Simple Setup (5 minutes)

### Step 1: Create Google Apps Script
1. Go to [script.google.com](https://script.google.com)
2. Click "New Project"
3. Delete the default code and paste the contents of `google_sheets_script.js`
4. Click "Save" (Ctrl+S)
5. Click "Deploy" → "New Deployment"
6. Choose "Web app" as the type
7. Set "Execute as" to "Me"
8. Set "Who has access" to "Anyone"
9. Click "Deploy"
10. Copy the Web App URL (it looks like: `https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec`)

### Step 2: Update ESP32 Code
1. Open `src/WebDashboard.cpp`
2. Find line 8: `const char* WebDashboard::GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec";`
3. Replace `YOUR_SCRIPT_ID` with the actual script ID from your URL
4. Save and upload to ESP32

### Step 3: Test
1. Run a few cycles on your router
2. Check your Google Drive for "Router Production Data" spreadsheet
3. Data will appear automatically every 30 seconds

## What You Get

- **Automatic backup**: All production data saved to Google Sheets
- **Real-time monitoring**: Data updates every 30 seconds
- **Historical tracking**: Complete production history
- **Easy access**: View data from anywhere
- **Free**: No cost, unlimited storage
- **Professional reports**: Use Google Sheets features for analysis

## Data Stored

Each row contains:
- Timestamp
- Total cycles
- Current hour cycles
- Hour, Day, Month, Year
- 1-minute average
- 5-minute average
- 15-minute average
- 30-minute average

## Troubleshooting

- **No data appearing**: Check the script URL in the code
- **Permission errors**: Make sure the script is deployed as "Anyone"
- **WiFi issues**: ESP32 will retry automatically when connection is restored

That's it! Your router data is now safely stored in the cloud forever.
