<configure-app>
	<configure-appearance></configure-appearance>

	<configure-notifications></configure-notifications>

	<configuration-container title="Complications">
		<complication-chooser-list/>
	</configuration-container>

	<div class='item-container'>
		<div class='button-container'>
			<input type='button' class='item-button' value='SAVE' onclick={ emitsave }>
		</div>
	</div>

	<script>
	emitsave() {
		var pack = {};
		for(tagName in this.tags) {
			var tag = this.tags[tagName];
			if(tag.to_json) {
				tag.to_json(pack);
			} else {
				console.log(tag, "contains no to_json!");
			}
		}
		console.log('done getting pack:', pack);
		this.trigger('save');
	}
	</script>
</configure-app>

<configure-appearance>
	<configuration-container title="Appearance">
		<configuration-content>
			<configuration-toggle check={ parent.parent.show_second_hand }>
				Second hand
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Display the second hand.  Note that this may increase battery consumption.
		</configuration-footer>
		<configuration-content>
			<configuration-toggle check={ parent.parent.show_no_connection }>
				"No connection" icon
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Display a "no connection" icon when the watch is disconnected from the phone.
		</configuration-footer>
	</configuration-container>

	<script>
	this.show_second_hand = false;
	this.show_no_connection = true;

	from_json(pack) {
		this.show_second_hand = pack['show_second_hand'];
		this.show_no_connection = pack['show_no_connection'];
	}

	to_json(pack) {
		pack['show_second_hand'] = this.show_second_hand;
		pack['show_no_connection'] = this.show_no_connection;
	}
	</script>
</configure-appearance>

<configure-notifications>
	<configuration-container title='Notifications'>
		<configuration-content>
			<configuration-toggle check={ parent.parent.vibrate_on_hour }>
				Vibrate on the hour
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Briefly vibrate once at the top of every hour, and twice at noon and midnight.
		</configuration-footer>
		<configuration-content>
			<configuration-toggle check={ parent.parent.vibrate_on_disconnect }>
				Vibrate on disconnect
			</configuration-toggle>
		</configuration-content>
		<configuration-footer>
			Vibrate when the watch loses its Bluetooth connection to the phone.
		</configuration-footer>
	</configuration-container>

	<script>
	this.vibrate_on_hour = false;
	this.vibrate_on_disconnect = false;

	from_json(pack) {
		this.vibrate_on_hour = pack["vibrate_on_hour"];
		this.vibrate_on_disconnect = pack["vibrate_on_disconnect"];
	}

	to_json(pack) {
		pack['vibrate_on_hour'] = this.vibrate_on_hour;
		pack['vibrate_on_disconnect'] = this.vibrate_on_disconnect;
	}
	</script>
</configure-notifications>
