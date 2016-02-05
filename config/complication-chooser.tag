<complication-chooser-list>
	<div class='item-container-header'>Complications</div>
	<complication-chooser each={ names } name={ name }/>

	function makeName(n) { return { name: n }; }
	this.names = [ 'Left', 'Bottom', 'Right' ].map(makeName);
</complication-chooser-list>

<complication-chooser>
	<div class='item-container-content'>
		<label class="item">
			<span>{ opts.name }</span>
			<select dir='rtl' class="item-select">
				<option class="item-select-option">None</option>
				<option class="item-select-option">Battery</option>
				<option class="item-select-option">Health</option>
				<option class="item-select-option">Date</option>
				<option class="item-select-option">Weather</option>
			</select>
		</label>
		<div id='main'></div>
	</div>

	riot.mount('div#main', 'complication-customize', {foo: "bar"});
</complication-chooser>

<complication-customize>
	<div>
		Hello customize of type { opts.foo }!
	</div>
</complication-customize>
