//api from https://api.sunrise-sunset.org/

const request = require("request-promise");
const moment = require('moment');

exports.handler = async (event, context) => {

    var body = await request('https://api.sunrise-sunset.org/json?lat=51.103499586&lng=3.737330384&formatted=0', { json: true });
    
    var astronomical_twilight_begin = moment(body.results.astronomical_twilight_begin).utc().add(1, 'hour');
    astronomical_twilight_begin = Math.round((astronomical_twilight_begin.hour() + astronomical_twilight_begin.minute() / 60) * 2);
    
    var nautical_twilight_begin = moment(body.results.nautical_twilight_begin).utc().add(1, 'hour');
    nautical_twilight_begin = Math.round((nautical_twilight_begin.hour() + nautical_twilight_begin.minute() / 60) * 2);

    var civil_twilight_begin = moment(body.results.civil_twilight_begin).utc().add(1, 'hour');
    civil_twilight_begin = Math.round((civil_twilight_begin.hour() + civil_twilight_begin.minute() / 60) * 2);

    var sunrise = moment(body.results.sunrise).utc().add(1, 'hour');
    sunrise = Math.round((sunrise.hour() + sunrise.minute() / 60) * 2);

    var sunset = moment(body.results.sunset).utc().add(1, 'hour');
    sunset = Math.round((sunset.hour() + sunset.minute() / 60) * 2);

    var civil_twilight_end = moment(body.results.civil_twilight_end).utc().add(1, 'hour');
    civil_twilight_end = Math.round((civil_twilight_end.hour() + civil_twilight_end.minute() / 60) * 2);
    
    var nautical_twilight_end = moment(body.results.nautical_twilight_end).utc().add(1, 'hour');
    nautical_twilight_end = Math.round((nautical_twilight_end.hour() + nautical_twilight_end.minute() / 60) * 2);

    var astronomical_twilight_end = moment(body.results.astronomical_twilight_end).utc().add(1, 'hour');
    astronomical_twilight_end = Math.round((astronomical_twilight_end.hour() + astronomical_twilight_end.minute() / 60) * 2);

    var resp = ''
    for (var i=0; i<48; i++)
    {
        console.log(i)
        if (i < astronomical_twilight_begin)
            resp=resp.concat('0')
        else if (i < nautical_twilight_begin)
            resp =resp.concat('1')
        else if (i < civil_twilight_begin)
            resp =resp.concat('2')
        else if (i < civil_twilight_end)
            resp = resp.concat('3')
        else if (i < nautical_twilight_end)
            resp = resp.concat('2')
        else if (i < astronomical_twilight_end)
            resp = resp.concat('1')
        else
            resp =resp.concat('0')
    }
   
    return {
        statusCode: 200,
        body: resp
    };
};
