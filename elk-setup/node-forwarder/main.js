const http = require('http')
      axios = require('axios'),
      exec = require('child_process').exec;

const IP = process.env.DNS || `http://[aaaa::212:7402:2:202]/`

const hostname = '0.0.0.0';
const port = 3000;

const server = http.createServer((req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.end('Hello World');
});

var interval = setInterval(() => {
  exec(`curl -H "User-Agent: curl" -H "Accept: */*" ${IP}`, (err, stdout, stderr) => {
  if(stdout.trim()) 
  {
    console.log(`Data: ${stdout}`);
    var moteIP = stdout.substring(0,stdout.indexOf(','));
    var tokens = stdout.substring(stdout.indexOf(',')+1).split(';');

    tokens.forEach(token => 
      {
        if (token == "")
        {
          console.log("break");
          
        }
        else 
        {
          var postData = moteIP + ',' + token;
          console.log(`PostData: ${postData}`);
          axios.post('https://logstash.alexanderrasmussen.dk', postData)
          .then(response => {
            console.log("Logstash response: ", response.data);
          })
          .catch(function (error) {
            console.log(error);
          });
        }
      });    
  }
  else {
    console.log(`Error: ${err}`);
    console.log(`StdError: ${stderr}`);
  }
  })
}, 10000);

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});