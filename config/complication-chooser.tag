<complication-chooser-list>
	<div class='item-container-header'>Complications</div>
	<complication-chooser each={ names } position={ p }/>

	function makeName(s) { return { p: s }; }
	this.names = [ 'Left', 'Bottom', 'Right' ].map(makeName);
</complication-chooser-list>

<complication-chooser>
	<div class='item-container-content'>
		<label class="item">
			<span>{ opts.position }</span>
			<select dir='rtl' class="item-select" name='typechooser' onchange={ ontypechosen } value={ chosentype }>
				<option class="item-select-option">None</option>
				<option class="item-select-option">Battery</option>
				<option class="item-select-option">Health</option>
				<option class="item-select-option">Date</option>
				<option class="item-select-option">Weather</option>
			</select>
		</label>
		<complication-customize name='customizer' />
	</div>

	<script>
	changecustomizer() {
		// Get the correct name of the customizer, or use the empty one
		var whichComplication = {
			"Battery": undefined,
			"Health": "complication-customize-health",
			"Date": undefined,
			"Weather": "complication-customize-weather"
		}[this.chosentype] || "complication-customize";

		// Mount it
		riot.mount(this.tags['customizer'].root, whichComplication, { foo: opts.cname});
	}

	ontypechosen(e) {
		this.chosentype = e.target.value;
		this.changecustomizer();
	}

	this.chosentype = "Health";
	this.on('mount', this.changecustomizer);
	</script>
</complication-chooser>

<complication-customize>
</complication-customize>

<complication-customize-health>
</complication-customize-health>

<complication-customize-weather>
</complication-customize-weather>
