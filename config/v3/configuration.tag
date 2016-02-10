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
	// complication-chooser-list is a sub-tag, so make a list of all the savables
	this.modules = $.extend({}, this.tags);

	delete this.modules["configuration-container"];
	this.modules["complication-chooser-list"] = this.tags["configuration-container"].tags["complication-chooser-list"];
	emitsave() {
		var pack = {};
		for (tagName in this.modules) {
			var tag = this.modules[tagName];
			if (tag.to_json) {
				tag.to_json(pack);
			} else {
				console.log(tag, "contains no to_json!");
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
				console.log(tag, "contains no from_json!");
			}
		}
	}
	this.on('load', this.from_json);
	</script>
</configure-app>

<configure-appearance>
	<configuration-container title="Appearance">
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

	this.show_second_hand = new this.Attribute(false);
	this.show_no_connection = new this.Attribute(true);

	from_json(pack) {
		this.show_second_hand.set(pack['show_second_hand']);
		this.show_no_connection.set(pack['show_no_connection']);
		this.update();
	}

	to_json(pack) {
		pack['show_second_hand'] = this.show_second_hand.get();
		pack['show_no_connection'] = this.show_no_connection.get();
	}
	</script>
</configure-appearance>

<configure-notifications>
	<configuration-container title='Notifications'>
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
		<configuration-footer>
			Vibrate when the watch loses its Bluetooth connection to the phone.
		</configuration-footer>
	</configuration-container>

	<script>
	this.mixin(Attribute);

	this.vibrate_on_hour = new this.Attribute(false);
	this.vibrate_on_disconnect = new this.Attribute(false);

	from_json(pack) {
		this.vibrate_on_hour.set(pack["vibrate_on_hour"]);
		this.vibrate_on_disconnect.set(pack["vibrate_on_disconnect"]);
		this.update();
	}

	to_json(pack) {
		pack['vibrate_on_hour'] = this.vibrate_on_hour.get();
		pack['vibrate_on_disconnect'] = this.vibrate_on_disconnect.get();
	}
	</script>
</configure-notifications>
