var moment = require('moment');

var offset = 0;

exports.handler = async (event, context) => {
    
    //offset++; //comment this rule to move to normal time. With this rule the clock simulates faster movement
    const response = moment.utc().add(1,'hour').add(offset*30,'minute').format('HHmm');

    return {
        statusCode: 200,
        body: `${response}`
    };
};