node-launchd
============

A Node.js Addon to get sockets from launchd upon (port-activated) launching on osx

Usage
----
Suppose, you created an app using express in directory

    /Users/you/Projects/yourapp

, and your node installation lives in

    /Users/you/Applications/Node.app/Contents

and you want your app to use port 8080.

First, create a launchd configuration file (e.g., yourapp.plist) in

	~/Library/LaunchAgents

, fill it with content such as this

    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
    <dict>
    	<key>Label</key>
    	<string>org.something.yourapp</string>
    	<key>EnvironmentVariables</key>
    		<dict>
    			<key>NODE_PATH</key>
    			<string>/Users/you/Applications/Node.app/Contents/lib/node_modules</string>
    		</dict>
    	<key>ProgramArguments</key>
    	<array>
    		<string>/Users/you/Applications/Node.app/Contents/bin/node</string>
    		<string>/Users/you/Projects/yourapp/app.js</string>
    	</array>
    	<key>Sockets</key>
    	<dict>
    		<key>HTTPSocket</key>
    		<dict>
    			<key>SockFamily</key>
    			<string>IPv4</string>
    			<key>SockServiceName</key>
    			<string>8080</string>
    			<key>SockType</key>
    			<string>stream</string>
    			<key>SockPassive</key>
    			<string>false</string>
    		</dict>
    	</dict>
    </dict>
    </plist>

change the app.js file in you project directory to start the http server using a launchd file handle like this

    http.createServer(app).listen({fd:require('node-launchd').getSocketFileDescriptorForName('HTTPSocket')});

Finally, you can load the launchd configuration file like this

    launchctl load -w ~/Library/LaunchAgents/yourapp.plist

