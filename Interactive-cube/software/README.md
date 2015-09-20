Interactive Cube Software
=========================

* Set Computer IP to 192.168.1.100

* Turn on Riot

* Connect touch screen midi thingy

* Start the Pd patch, make sure midi and OSC works
    `cd Pd-Patches`
    `/Applications/Pd-extended.app/Contents/MacOS/Pd-extended main2.pd`

* Start the OSC proxy
    `cd oscproxy`
    `npm install`
    `node osc-proxy.js`

* Start web server
    `cd browser`
    `python -m SimpleHTTPServer`
    `open http://localhost:8000/drumcube.html`
