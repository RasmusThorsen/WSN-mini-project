const http = require('http')
      axios = require('axios'),
      exec = require('child_process').exec;

const hostname = '0.0.0.0';
const port = 3000;

const server = http.createServer((req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.end('Hello World');
});

// var interval = setInterval(() => {
//   axios.get('http://[aaaa::212:7402:2:202]/', { headers: { 'User-Agent': 'axios', 'Accept': '*/*' } })
//     .then(response => {
//       console.log('Data from motes: ', response.data)
//       axios.post('https://logstash.alexanderrasmussen.dk', response.data)
//         .then(response => {
//           console.log("Logstash response: ", response.data);
//         });
//     }).catch(error => {
//       console.log(error);
//     })
// }, 10000);

// let optionsMote = {
//   host: '[aaaa::212:7402:2:202]',
//   hostname: 'aaaa::212:7402:2:202',
//   port: 80,
//   path: '/',
//   method: 'GET',
//   headers: {
//     'Accept': '*/*',
//     'User-Agent': 'node'
//   }
// }

// var interval = setInterval(() => {
//   const req = http.request(optionsMote, res => {
//     console.log(`statusCode: ${res}`)

//     res.on('data', d => {
//       console.log(`Data. ${d}`)
//     })
//   })

//   req.on('error', error => {
//     console.error(error)
//   })

//   req.end()
// }, 10000);

var interval = setInterval(() => {
  exec('curl -H "User-Agent: curl" -H "Accept: */*" http://[aaaa::212:7402:2:202]/', (err, stdout, stderr) => {
  if(stdout.trim()) 
  {
    console.log(`Data: ${stdout}`);
    axios.post('https://logstash.alexanderrasmussen.dk', stdout)
      .then(response => {
        console.log("Logstash response: ", response.data);
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