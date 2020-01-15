var moment = require('moment');

exports.handler = async (event, context) => {
    
    const response = moment.utc().add(1,'hour').format('HHmm');

    return {
        statusCode: 200,
        body: `${response}`
    };
};