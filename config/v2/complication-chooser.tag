<complication-customize>
</complication-customize>

<complication-customize-health>
	<div> foo </div>
</complication-customize-health>

<complication-customize-weather>
</complication-customize-weather>

<complication-chooser>
	<configuration-content name='content'>
		<configuration-dropdown caption={ parent.opts.position } attrib={ parent.chosentype }>
			<option class="item-select-option">None</option>
			<option class="item-select-option">Battery</option>
			<option class="item-select-option">Health</option>
			<option class="item-select-option">Date</option>
			<option class="item-select-option">Weather</option>
		</configuration-dropdown>

		<complication-customize name='customizer' />
	</configuration-content>

	<script>
	this.mixin(Attribute);
	var self = this;

	this.chosentype = new this.Attribute("None");

	changecustomizer(val) {
		// Get the correct name of the customizer, or use the empty one
		var whichComplication = {
			"Battery": undefined,
			"Health": "complication-customize-health",
			"Date": undefined,
			"Weather": "complication-customize-weather"
		}[val] || "complication-customize";

		// Mount it
		riot.mount(self.tags['content'].tags['customizer'].root, whichComplication, { foo: opts.cname});
	}
	this.chosentype.onSet = this.changecustomizer;
	this.on('mount', this.changecustomizer);

	from_json(pack) {
		this.chosentype.set(pack["type"]);

		// We need to keep a reference in order to configure the "saved" type
		this.configPack = pack;

		this.update();
	}

	to_json(pack) {
		pack["type"] = this.chosentype.get();
		pack["options"] = {};
		if(this.tags['content'].tags['customizer'].to_json) {
			this.tags['content'].tags['customizer'].to_json(pack["options"]);
		}
	}
	</script>
</complication-chooser>

<complication-chooser-list>
	<complication-chooser each={ names } position={ p }/>

	<script>
	function makeName(s) { return { p: s }; }
	this.names = [ 'Left', 'Bottom', 'Right' ].map(makeName);

	from_json(pack) {
		var allComplications = pack["complications"];
		if(allComplications instanceof Object) {
			for(i in this.tags['complication-chooser']) {
				var tag = this.tags['complication-chooser'][i];
				tag.from_json(allComplications[tag.opts.position]);
			}
		}
	}

	to_json(pack) {
		var allComplications = {};
		for(i in this.tags['complication-chooser']) {
			var tag = this.tags['complication-chooser'][i];
			var complication_pack = {};
			tag.to_json(complication_pack);
			allComplications[tag.opts.position] = complication_pack;
		}
		pack["complications"] = allComplications;
	}
	</script>
</complication-chooser-list>

