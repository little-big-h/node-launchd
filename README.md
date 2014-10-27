node-launchd
============

A Node.js add-on to get sockets from launchd upon (port-activated) launching on OS X.

Usage
-----

Suppose, you created an app using Express in the directory

    /Users/you/Projects/yourapp

and your node installation is installed in the prefix

    /usr/local

and you want your app to use port 8080.

First, create a launchd configuration file (e.g., com.example.yourapp.plist) in

    ~/Library/LaunchAgents

Fill it with content such as this

    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
    <dict>
    	<key>EnvironmentVariables</key>
    	<dict>
    		<key>NODE_ENV</key>
    		<string>production</string>
    	</dict>
    	<key>Label</key>
    	<string>com.example.yourapp</string>
    	<key>ProgramArguments</key>
    	<array>
    		<string>/usr/local/bin/node</string>
    		<string>app.js</string>
    	</array>
    	<key>Sockets</key>
    	<dict>
    		<key>HTTPSocket</key>
    		<dict>
    			<key>SockFamily</key>
    			<string>IPv4v6</string>
    			<key>SockServiceName</key>
    			<integer>8080</integer>
    		</dict>
    	</dict>
    	<key>WorkingDirectory</key>
    	<string>/Users/you/Projects/yourapp</string>
    </dict>
    </plist>

If using OS X 10.9 or earlier, change `SockFamily` to `IPv4` (or `IPv6`) instead of `IPv4v6`.

Change the app.js file in your project directory to start the http server using a launchd file handle like this

    http.createServer(app).listen({fd:require('node-launchd').getSocketFileDescriptorForName('HTTPSocket')});

Finally, you can load the launchd configuration file like this

    launchctl load -w ~/Library/LaunchAgents/com.example.yourapp.plist

