riot.tag2('complication-customize', '', '', '', function(opts) {
});

riot.tag2('complication-customize-health', '', '', '', function(opts) {
});

riot.tag2('complication-customize-weather', '', '', '', function(opts) {
});

riot.tag2('complication-chooser', '<configuration-content name="content"> <configuration-dropdown caption="{parent.opts.position}" selectname="typechooser" onchange="{parent.ontypechosen}" value="{parent.chosentype}"> <option class="item-select-option">None</option> <option class="item-select-option">Battery</option> <option class="item-select-option">Health</option> <option class="item-select-option">Date</option> <option class="item-select-option">Weather</option> </configuration-dropdown> <complication-customize name="customizer"></complication-customize> </configuration-content>', '', '', function(opts) {
	this.changecustomizer = function() {

		var whichComplication = {
			"Battery": undefined,
			"Health": "complication-customize-health",
			"Date": undefined,
			"Weather": "complication-customize-weather"
		}[this.chosentype] || "complication-customize";

		riot.mount(this.tags['content'].tags['customizer'].root, whichComplication, { foo: opts.cname});
	}.bind(this)

	this.ontypechosen = function(e) {
		this.chosentype = e.target.value;
		this.changecustomizer();
	}.bind(this)

	this.chosentype = "Health";
	this.on('mount', this.changecustomizer);
}, '{ }');

riot.tag2('complication-chooser-list', '<div class="item-container-header">Complications</div> <complication-chooser each="{names}" position="{p}"></complication-chooser>', '', '', function(opts) {
	function makeName(s) { return { p: s }; }
	this.names = [ 'Left', 'Bottom', 'Right' ].map(makeName);
}, '{ }');

