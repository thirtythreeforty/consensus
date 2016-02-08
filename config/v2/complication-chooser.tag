<complication-customize>
</complication-customize>

<complication-customize-health>
</complication-customize-health>

<complication-customize-weather>
	<configuration-content>
		<configuration-dropdown caption='Gadget' attrib={ parent.gadget }>
			<option class="item-select-option" value='icon'>Conditions (icon)</configuration-option>
			<option class="item-select-option" value='degF'>Temperature (&deg;F)</configuration-option>
			<option class="item-select-option" value='degC'>Temperature (&deg;C)</configuration-option>
			<option class="item-select-option" value='relhum'>Humidity (%)</configuration-option>
		</configuration-dropdown>
	</configuration-content>
	<configuration-footer>
		Select the gadget displayed in the center of the complication's ring.
	</configuration-footer>

	<script>
	this.mixin(Attribute);
	this.gadget = new this.Attribute("icon");

	from_json(pack) {
		this.gadget.set(pack["gadget"]);
		this.update();
	}
	to_json(pack) {
		pack["gadget"] = this.gadget.get();
	}
	</script>
</complication-customize-weather>

<complication-chooser>
	<configuration-content name='content'>
		<configuration-dropdown style="font-weight:bolder;" caption="{ parent.opts.position }" attrib={ parent.chosentype }>
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
	this.configPack = {options: {}};
	this.customizer = null;

	configureCustomizer() {
		if(self.chosentype.get() === self.configPack["type"] && self.customizer.from_json) {
			self.customizer.from_json(self.configPack["options"]);
		}
	}

	changecustomizer(val) {
		// Get the correct name of the customizer, or use the empty one
		var whichComplication = {
			"None": undefined,
			"Battery": undefined,
			"Health": "complication-customize-health",
			"Date": undefined,
			"Weather": "complication-customize-weather"
		}[val] || "complication-customize";

		// Mount it
		self.customizer = riot.mount(self.tags['content'].tags['customizer'].root, whichComplication, {})[0];

		// Configure it
		self.configureCustomizer();
	}
	this.chosentype.onSet = this.changecustomizer;
	this.on('mount', this.changecustomizer);

	from_json(pack) {
		this.chosentype.set(pack["type"]);

		// We need to keep a reference in order to configure the "saved" customizer type
		this.configPack = pack;

		this.configureCustomizer();

		this.update();
	}

	to_json(pack) {
		pack["type"] = this.chosentype.get();
		pack["options"] = {};
		if(this.customizer.to_json) {
			this.customizer.to_json(pack["options"]);
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

