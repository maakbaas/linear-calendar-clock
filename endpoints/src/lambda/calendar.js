require('dotenv').config()
var moment = require('moment');

const { google } = require('googleapis')

// const MAX_RESULTS = process.env.MAX_RESULTS
const CALENDAR_ID = process.env.CALENDAR_ID
const CLIENT_ID = process.env.CLIENT_ID
const CLIENT_SECRET = process.env.CLIENT_SECRET
const REDIRECT_URI = process.env.REDIRECT_URI

const TOKEN = {
    access_token: process.env.ACCESS_TOKEN,
    scope: "https://www.googleapis.com/auth/calendar.readonly",
    token_type: "Bearer",
    refresh_token: process.env.REFRESH_TOKEN,
    expiry_date: process.env.EXPIRY_DATE
}

const Authorize = async () => {
    let oAuth2Client = await new google.auth.OAuth2(CLIENT_ID, CLIENT_SECRET, REDIRECT_URI)
    oAuth2Client.setCredentials(TOKEN)
    return await google.calendar({
        version: 'v3',
        auth: oAuth2Client
    })
}

const getEvents = async ({date = null }) => {
    
    var response = '000000000000000000000000000000000000000000000000';
    const calendar = await Authorize()

    const list = await calendar.calendarList.list()

    for (var n = 0; n < list.data.items.length; n++)
    {
        let _query = {
            calendarId: list.data.items[n].id,
            timeMin: (new Date()).toISOString(),
            maxResults: 100,
            singleEvents: true,
            orderBy: 'startTime'
        }

        _query.timeMin = `${date.replace(/\"/g, '')}T05:00:00.000Z`
        _query.timeMax = `${date.replace(/\"/g, '')}T23:30:00-05:00`
        _query.maxResults = 100 || 1   

        const res = await calendar.events.list(_query)

        const events = res.data.items
        const calEvents = {
            success: true,
            message: 'success',
            events: []
        }
        if (events.length) {
            calEvents.events = events.map(event => ({
                id: event.id,
                status: event.status,
                iCalUID: event.iCalUID,
                recurringEventId: event.recurringEventId,
                htmlLink: event.htmlLink,
                //extendedPropertiesShared: event.extendedProperties.shared,
                //extendedPropertiesPrivate: event.extendedProperties.private,
                start: event.start.dateTime || event.start.date,
                end: event.end.dateTime || event.end.date,
                summary: event.summary || '',
                description: event.description || ''
            }))
        } else {
            calEvents.events = "none"
        }

        if (calEvents.events != "none") {
            for (var i = 0; i < Object.keys(calEvents.events).length; i++) {
                var starttime = moment.utc(calEvents.events[i].start).add(1, 'hour');
                var stoptime = moment.utc(calEvents.events[i].end).add(1, 'hour');
                starttime = parseFloat(starttime.format('HH')) + parseFloat(starttime.format('mm')) / 60;
                stoptime = parseFloat(stoptime.format('HH')) + parseFloat(stoptime.format('mm')) / 60;

                for (var j = 0; j < 48; j++) {
                    //still need to adapt for multiday appointments
                    if ((starttime >= (j / 2) && starttime < ((j + 1) / 2)) || //startindex in window
                        (stoptime > (j / 2) && stoptime <= ((j + 1) / 2)) || //stopindex in window
                        (starttime < (j / 2) && stoptime > (j / 2)))
                        response = response.substr(0, j) + (n+1).toString() + response.substr(j + 1);
                }

            }
        }

    }    

    return response
}

exports.handler = (event, context, callback) => {

    let date = moment.utc().add(1, 'hour').format('YYYY-MM-DD');  

    getEvents({ date }).then(res => {            

            callback(null, {
                statusCode: 200,
                body: res
            })
        })
        .catch(e => {
            callback(e)
        })
}