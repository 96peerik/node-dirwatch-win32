#node-dirwatch-win32
Windows-only directory watching node.js module

###Install
npm install node-dirwatch-win32

--

To build this library, following components are needed:

* Visual Studio 2015 with c++ installed

* Python 2.7

* Python environment variable PYTHON={python-dir including exe}


###Example code

```
#!javascript

const DW = require('node-dirwatch-win32').DirectoryWatcher;
let dw = new DW('c:\\chyronhego');
dw.on('change', () => {
  console.log('Something changed');
});

dw.on('error', (err) => {
  console.error(err.message);
});

