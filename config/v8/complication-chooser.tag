<complication-customize>
</complication-customize>

<complication-customize-date>
	<configuration-content>
		<configuration-dropdown caption='Gadget' attrib={ parent.gadget }>
			<option class="item-select-option" value='day'>Day of Week</configuration-option>
			<option class="item-select-option" value='month'>Month</configuration-option>
			<option class="item-select-option" value='none'>None</configuration-option>
		</configuration-dropdown>
	</configuration-content>
	<configuration-footer>
		Select the gadget displayed alongside the current date.
	</configuration-footer>

	<script>
	this.mixin(Attribute);
	this.gadget = new this.Attribute('day');

	from_json(pack) {
		this.gadget.set(pack["gadget"]);
		this.update();
	}
	to_json(pack) {
		pack["gadget"] = this.gadget.get();
	}
	</script>
</complication-customize-date>

<complication-customize-health>
	<configuration-content>
		<configuration-dropdown caption='Gadget' attrib={ parent.gadget }>
			<option class="item-select-option" value='icon'>Status icon</configuration-option>
			<option class="item-select-option" value='distance'>Distance walked</configuration-option>
			<option class="item-select-option" value='kcal_act'>KCal active</configuration-option>
			<option class="item-select-option" value='kcal_rest'>KCal resting</configuration-option>
			<option class="item-select-option" value='kcal_tot'>KCal total</configuration-option>
			<option class="item-select-option" value='act_sec'>Time active</configuration-option>
		</configuration-dropdown>
	</configuration-content>
	<configuration-footer>
		Select the gadget displayed in the center of the complication's ring.
	</configuration-footer>
	<configuration-content>
		<configuration-dropdown caption='Goal Type' attrib={ parent.goal_type }>
			<option class="item-select-option" value='auto'>Auto</configuration-option>
			<option class="item-select-option" value='manual'>Manual</configuration-option>
		</configuration-dropdown>
	</configuration-content>
	<configuration-content name='goalinput'>
		<configuration-input input_type="number" attrib={ parent.goal }/>
	</configuration-content>
	<configuration-footer>
		Choose your daily step goal, either by specifying it yourself or (recommended) letting Pebble calculate it for you.
	</configuration-footer>

	<script>
	var self = this;

	this.mixin(Attribute);
	this.gadget = new this.Attribute("icon");
	this.goal_type = new this.Attribute("auto", function(val) {
		self.tags['goalinput'].root.style.display =
			(val === 'manual') ? "initial" : "none";
	});
	this.goal = new this.Attribute("10000");

	from_json(pack) {
		this.gadget.set(pack["gadget"]);
		this.goal_type.set(pack["goal_type"]);
		this.goal.set(pack["goal"]);
		this.update();
	}
	to_json(pack) {
		pack["gadget"] = this.gadget.get();
		pack["goal_type"] = this.goal_type.get();
		pack["goal"] = this.goal.get();
	}
	</script>
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

		<img src="weather-explanation.svg" style="width:100%;">
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

<complication-customize-timezone>
	<configuration-content>
		<configuration-dropdown caption='UTC Offset' attrib={ parent.offset }>
			<option class="item-select-option" value='-12'>UTC&minus;12:00</configuration-option>
			<option class="item-select-option" value='-11'>UTC&minus;11:00</configuration-option>
			<option class="item-select-option" value='-10'>UTC&minus;10:00</configuration-option>
			<option class="item-select-option" value='-9.5'>UTC&minus;09:30</configuration-option>
			<option class="item-select-option" value='-9'>UTC&minus;09:00</configuration-option>
			<option class="item-select-option" value='-8'>UTC&minus;08:00</configuration-option>
			<option class="item-select-option" value='-7'>UTC&minus;07:00</configuration-option>
			<option class="item-select-option" value='-6'>UTC&minus;06:00</configuration-option>
			<option class="item-select-option" value='-5'>UTC&minus;05:00</configuration-option>
			<option class="item-select-option" value='-4.5'>UTC&minus;04:30</configuration-option>
			<option class="item-select-option" value='-4'>UTC&minus;04:00</configuration-option>
			<option class="item-select-option" value='-3.5'>UTC&minus;03:30</configuration-option>
			<option class="item-select-option" value='-3'>UTC&minus;03:00</configuration-option>
			<option class="item-select-option" value='-2'>UTC&minus;02:00</configuration-option>
			<option class="item-select-option" value='-1'>UTC&minus;01:00</configuration-option>
			<option class="item-select-option" value='0'>UTC&plusmn;00:00</configuration-option>
			<option class="item-select-option" value='1'>UTC+01:00</configuration-option>
			<option class="item-select-option" value='2'>UTC+02:00</configuration-option>
			<option class="item-select-option" value='3'>UTC+03:00</configuration-option>
			<option class="item-select-option" value='3.5'>UTC+03:30</configuration-option>
			<option class="item-select-option" value='4'>UTC+04:00</configuration-option>
			<option class="item-select-option" value='4.5'>UTC+04:30</configuration-option>
			<option class="item-select-option" value='5'>UTC+05:00</configuration-option>
			<option class="item-select-option" value='5.5'>UTC+05:30</configuration-option>
			<option class="item-select-option" value='5.75'>UTC+05:45</configuration-option>
			<option class="item-select-option" value='6'>UTC+06:00</configuration-option>
			<option class="item-select-option" value='6.5'>UTC+06:30</configuration-option>
			<option class="item-select-option" value='7'>UTC+07:00</configuration-option>
			<option class="item-select-option" value='8'>UTC+08:00</configuration-option>
			<option class="item-select-option" value='8.5'>UTC+08:30</configuration-option>
			<option class="item-select-option" value='8.75'>UTC+08:45</configuration-option>
			<option class="item-select-option" value='9'>UTC+09:00</configuration-option>
			<option class="item-select-option" value='9.5'>UTC+09:30</configuration-option>
			<option class="item-select-option" value='10'>UTC+10:00</configuration-option>
			<option class="item-select-option" value='10.5'>UTC+10:30</configuration-option>
			<option class="item-select-option" value='11'>UTC+11:00</configuration-option>
			<option class="item-select-option" value='12'>UTC+12:00</configuration-option>
			<option class="item-select-option" value='12.75'>UTC+12:45</configuration-option>
			<option class="item-select-option" value='13'>UTC+13:00</configuration-option>
			<option class="item-select-option" value='14'>UTC+14:00</configuration-option>
	</configuration-content>
	<configuration-footer>
		Choose a UTC offset for the clock.  Note that due to ever-changing daylight savings time policies, this will not be adjusted for daylight savings time!
	</configuration-footer>

	<script>
	this.mixin(Attribute);
	this.offset = new this.Attribute("0");

	from_json(pack) {
		this.offset.set(pack["offset"]);
		this.update();
	}
	to_json(pack) {
		pack["offset"] = this.offset.get();
	}
	</script>
</complication-customize-timezone>

<complication-chooser>
	<configuration-content name='content'>
		<configuration-dropdown style="font-weight:bold;" caption="{ parent.opts.position }" attrib={ parent.chosentype }>
			<option class="item-select-option">None</option>
			<option class="item-select-option">Battery</option>
			<option class="item-select-option">Health</option>
			<option class="item-select-option">Date</option>
			<option class="item-select-option">Weather</option>
			<option class="item-select-option">Time Zone</option>
		</configuration-dropdown>
	</configuration-content>
	<complication-customize name='customizer' />

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
			"Date": "complication-customize-date",
			"Weather": "complication-customize-weather",
			"Time Zone": "complication-customize-timezone"
		}[val] || "complication-customize";

		// Mount it
		self.customizer = riot.mount(self.tags['customizer'].root, whichComplication, {})[0];

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

