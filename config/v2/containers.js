riot.tag2('configuration-container', '<div class="item-container"> <div class="item-container-header"> {opts.title} </div> <yield></yield> </div>', '', '', function(opts) {
}, '{ }');

riot.tag2('configuration-content', '<div class="item-container-content"> <yield></yield> </div>', '', '', function(opts) {
});

riot.tag2('configuration-footer', '<div class="item-container-footer"> <yield></yield> </div>', '', '', function(opts) {
});

riot.tag2('configuration-toggle', '<label class="item"> <yield></yield> <input id="{inputid}" type="checkbox" class="item-toggle"></input> </label>', '', '', function(opts) {


	this.one('mount', function() { $(this.root).find('.item-toggle').itemToggle(); });
}, '{ }');

riot.tag2('configuration-dropdown', '<label class="item"> {opts.caption} <select dir="rtl" class="item-select" name="{opts.selectname}" onchange="{opts.onchange}" value="{opts.value}"> <yield></yield> </select> </label>', '', '', function(opts) {
	this.one('mount', function() { $(this.root).find('.item-select').itemSelect(); });
}, '{ }');

riot.tag2('configuration-option', '<option class="item-select-option"> <yield></yield> </option>', '', '', function(opts) {
});
