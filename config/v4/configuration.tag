<configure-app>
	<configure-appearance></configure-appearance>

	<configure-behavior />

	<configuration-container name="compcont" title="Complications">
		<complication-chooser-list/>
	</configuration-container>

	<configuration-container title="Miscellaneous">
		<configuration-content>
		<configuration-footer>
			Consensus takes me lots of time to design and build. If you enjoy the watchface, please consider a small donation to buy me a coffee while I work on new features. Thanks for your support!
		</configuration-footer>
		<center>
			<!-- This blob is directly from PayPal, although target has been modified to "_blank" -->
			<form action="https://www.paypal.com/cgi-bin/webscr" method="post" target="_blank">
			<input type="hidden" name="cmd" value="_s-xclick">
			<input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----MIIHRwYJKoZIhvcNAQcEoIIHODCCBzQCAQExggEwMIIBLAIBADCBlDCBjjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRQwEgYDVQQKEwtQYXlQYWwgSW5jLjETMBEGA1UECxQKbGl2ZV9jZXJ0czERMA8GA1UEAxQIbGl2ZV9hcGkxHDAaBgkqhkiG9w0BCQEWDXJlQHBheXBhbC5jb20CAQAwDQYJKoZIhvcNAQEBBQAEgYBktkyEAON6NkazbAq1nCHT/iaqaITT8k5kgoWVZHFBDvcy0Dv30r8x+cMAXW1EIeAfQVwT6tiMrGm+21JQ7FYgHrgj1FQY7MA6dOcn38DsQNcD5Al5mKrbPhnTZpcodHyWpO4MbeEXnQll/K1PW4Kqsrq1TNkhdcssDHARkx6lKTELMAkGBSsOAwIaBQAwgcQGCSqGSIb3DQEHATAUBggqhkiG9w0DBwQILY1NOEcW+YuAgaA9hAarzvk5Avx4aV7CDZe+ZzD7nY3yZgPSn7tR08Cfzbv9GZjv+QHAvhsIeAM9LTFq60rx3gO/mMeoqqOAjNf6+dyYsPB6J5ODjTCic5Nqs2VCKjEWhL4y9RFQW77qs1A1zsmikZqOtf4fosnjXKE/t7R3sMz0ZJmt1VpC90lq7RBIkLa+sAZjprNehXifNgzUO67dxsvib4qHYR4tLj5PoIIDhzCCA4MwggLsoAMCAQICAQAwDQYJKoZIhvcNAQEFBQAwgY4xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEWMBQGA1UEBxMNTW91bnRhaW4gVmlldzEUMBIGA1UEChMLUGF5UGFsIEluYy4xEzARBgNVBAsUCmxpdmVfY2VydHMxETAPBgNVBAMUCGxpdmVfYXBpMRwwGgYJKoZIhvcNAQkBFg1yZUBwYXlwYWwuY29tMB4XDTA0MDIxMzEwMTMxNVoXDTM1MDIxMzEwMTMxNVowgY4xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEWMBQGA1UEBxMNTW91bnRhaW4gVmlldzEUMBIGA1UEChMLUGF5UGFsIEluYy4xEzARBgNVBAsUCmxpdmVfY2VydHMxETAPBgNVBAMUCGxpdmVfYXBpMRwwGgYJKoZIhvcNAQkBFg1yZUBwYXlwYWwuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDBR07d/ETMS1ycjtkpkvjXZe9k+6CieLuLsPumsJ7QC1odNz3sJiCbs2wC0nLE0uLGaEtXynIgRqIddYCHx88pb5HTXv4SZeuv0Rqq4+axW9PLAAATU8w04qqjaSXgbGLP3NmohqM6bV9kZZwZLR/klDaQGo1u9uDb9lr4Yn+rBQIDAQABo4HuMIHrMB0GA1UdDgQWBBSWn3y7xm8XvVk/UtcKG+wQ1mSUazCBuwYDVR0jBIGzMIGwgBSWn3y7xm8XvVk/UtcKG+wQ1mSUa6GBlKSBkTCBjjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRQwEgYDVQQKEwtQYXlQYWwgSW5jLjETMBEGA1UECxQKbGl2ZV9jZXJ0czERMA8GA1UEAxQIbGl2ZV9hcGkxHDAaBgkqhkiG9w0BCQEWDXJlQHBheXBhbC5jb22CAQAwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCBXzpWmoBa5e9fo6ujionW1hUhPkOBakTr3YCDjbYfvJEiv/2P+IobhOGJr85+XHhN0v4gUkEDI8r2/rNk1m0GA8HKddvTjyGw/XqXa+LSTlDYkqI8OwR8GEYj4efEtcRpRYBxV8KxAW93YDWzFGvruKnnLbDAF6VR5w/cCMn5hzGCAZowggGWAgEBMIGUMIGOMQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExFjAUBgNVBAcTDU1vdW50YWluIFZpZXcxFDASBgNVBAoTC1BheVBhbCBJbmMuMRMwEQYDVQQLFApsaXZlX2NlcnRzMREwDwYDVQQDFAhsaXZlX2FwaTEcMBoGCSqGSIb3DQEJARYNcmVAcGF5cGFsLmNvbQIBADAJBgUrDgMCGgUAoF0wGAYJKoZIhvcNAQkDMQsGCSqGSIb3DQEHATAcBgkqhkiG9w0BCQUxDxcNMTYwMjE3MDAyNTA4WjAjBgkqhkiG9w0BCQQxFgQUgvfZ4xgYGzGuwCVvfzIKT9ZcO/YwDQYJKoZIhvcNAQEBBQAEgYAp2ymy1BrI7cts6Ne6w9BRepgpC4bfD2p3XWfLOuuZ0s+kcIqjtvvlLhes7DA1dsdbaWOvd82t6Jacpj+4hmYk+IOTcTMRFegScckCfFg+9jkXQ80twf4xynnXJen3tGkMMBt0xavosEI8UjF30GNIKXWF74MYfacUKvN9YxTqkw==-----END PKCS7-----
			">
			<input type="image" src="https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!">
			</form>
		</center>
		</configuration-content>
	</configuration-container>

	<div class='item-container'>
		<div class='button-container'>
			<input type='button' class='item-button' value='SAVE' onclick={ emitsave }>
		</div>
	</div>

	<script>
	// complication-chooser-list is a sub-tag, so make a list of all the savables
	this.modules = $.extend({}, this.tags);

	delete this.modules["compcont"];
	this.modules["complication-chooser-list"] = this.tags["compcont"].tags["complication-chooser-list"];
	emitsave() {
		var pack = {};
		for (tagName in this.modules) {
			var tag = this.modules[tagName];
			if (tag.to_json) {
				tag.to_json(pack);
			} else {
				console.log(tag.toString(), "contains no to_json!");
			}
		}
		this.trigger('save', pack);
	}

	from_json(pack) {
		for (tagName in this.modules) {
			var tag = this.modules[tagName];
			if (tag.from_json) {
				tag.from_json(pack);
			} else {
				console.log(tag.toString(), "contains no from_json!");
			}
		}
	}
	this.on('load', this.from_json);
	</script>
</configure-app>

<configure-appearance>
	<configuration-container title="Appearance">
		<configuration-content>
			<configuration-dropdown caption='Theme' attrib={ parent.parent.theme }>
				<option class="item-select-option" value='bold-dark'>Bold Dark</configuration-option>
				<option class="item-select-option" value='thin'>Thin</configuration-option>
				<option class="item-select-option" value='bold-white'>Bold White</configuration-option>
			</configuration-dropdown>
		</configuration-content>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.show_second_hand }>
				Second hand
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Display the second hand.  Note that this may increase battery consumption.
		</configuration-footer>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.show_no_connection }>
				"No connection" icon
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Display a "no connection" icon when the watch is disconnected from the phone.
		</configuration-footer>
	</configuration-container>

	<script>
	this.mixin(Attribute);

	this.theme = new this.Attribute('bold-dark');
	this.show_second_hand = new this.Attribute(false);
	this.show_no_connection = new this.Attribute(true);

	from_json(pack) {
		this.theme.set(pack["theme"]);
		this.show_second_hand.set(pack['show_second_hand']);
		this.show_no_connection.set(pack['show_no_connection']);
		this.update();
	}

	to_json(pack) {
		pack['theme'] = this.theme.get();
		pack['show_second_hand'] = this.show_second_hand.get();
		pack['show_no_connection'] = this.show_no_connection.get();
	}
	</script>
</configure-appearance>

<configure-behavior>
	<configuration-container title='Behavior'>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.vibrate_on_hour }>
				Vibrate on the hour
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Briefly vibrate once at the top of every hour, and twice at noon and midnight.
		</configuration-footer>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.vibrate_on_disconnect }>
				Vibrate on disconnect
			</configuration-toggle>
		</configuration-content>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.vibrate_on_connect }>
				Vibrate on connect
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Vibrate when the watch gains or loses its Bluetooth connection to the phone.
		</configuration-footer>
		<configuration-content>
			<configuration-toggle attrib={ parent.parent.quiet_during_sleep }>
				Quiet while sleeping
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			When you are sleeping, disable all these vibrations (but don't worry: alarms and timers will still buzz). Has no effect if Pebble Health is disabled.
		</configuration-footer>
	</configuration-container>
	<configuration-content>
		<configuration-dropdown caption='Location' attrib={ parent.location_type }>
			<option class="item-select-option" value='auto'>Current</configuration-option>
			<option class="item-select-option" value='manual'>Manual</configuration-option>
		</configuration-dropdown>
	</configuration-content>
	<configuration-content name='location'>
		<configuration-input input_type="text" input_placeholder="ZIP/Postal Code" attrib={ parent.location }/>
	</configuration-content>
	<configuration-footer>
		Set the location to use when fetching weather data. If "Current," your phone's location services must be enabled.
	</configuration-footer>

	<script>
	var self = this;

	this.mixin(Attribute);

	this.vibrate_on_hour = new this.Attribute(true);
	this.vibrate_on_disconnect = new this.Attribute(true);
	this.vibrate_on_connect = new this.Attribute(false);
	this.quiet_during_sleep = new this.Attribute(false);
	this.location_type = new this.Attribute("auto", function(val) {
		self.tags['location'].root.style.display =
			(val === 'manual') ? "initial" : "none";
	});
	this.location = new this.Attribute("");

	from_json(pack) {
		this.vibrate_on_hour.set(pack["vibrate_on_hour"]);
		this.vibrate_on_disconnect.set(pack["vibrate_on_disconnect"]);
		this.vibrate_on_connect.set(pack["vibrate_on_connect"]);
		this.quiet_during_sleep.set(pack["quiet_during_sleep"]);
		this.location.set(pack["location"]);
		this.location_type.set(pack["location_type"]);
		this.update();
	}

	to_json(pack) {
		pack['vibrate_on_hour'] = this.vibrate_on_hour.get();
		pack['vibrate_on_disconnect'] = this.vibrate_on_disconnect.get();
		pack['vibrate_on_connect'] = this.vibrate_on_connect.get();
		pack['quiet_during_sleep'] = this.quiet_during_sleep.get();
		pack["location"] = this.location.get();
		pack["location_type"] = this.location_type.get();
	}
	</script>
</configure-behavior>
