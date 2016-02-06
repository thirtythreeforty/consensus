var Injection = {
	injectOnce: function(toFind, toExclude, func) {
		// Close over 'readied'
		var readied = false;
		this.one('mount', function() {
			if(!readied) {
				func.call($(this.root).find(toFind).not('*>'+toExclude));
				console.log("annotated!");
			} else {
				console.log("ready already fired, not re-annotating");
			}
		});
		$(function() { readied = true; console.log("ready called"); });
	}
}

<configuration-container>
	<div class='item-container'>
		<div class='item-container-header'>
			{ opts.title }
		</div>

		<yield />
	</div>
</configuration-container>

<configuration-content>
	<div class='item-container-content'>
		<yield />
	</div>
</configuration-content>

<configuration-footer>
	<div class='item-container-footer'>
		<yield />
	</div>
</configuration-footer>

<configuration-toggle>
	<label class='item'>
		<yield />
		<!-- TODO inputid is not going to work -->
		<input id={ inputid } type='checkbox' class='item-toggle'></input>
	</label>

	<script>
	this.mixin(Injection);
	this.injectOnce('.item-toggle', '.item-style-toggle', $.fn.itemToggle);
	</script>
</configuration-toggle>

<configuration-dropdown>
	<label class="item">
		{ opts.caption }
		<select dir='rtl' class='item-select' name={ opts.selectname } onchange={ opts.onchange} value={ opts.value }>
			<yield />
		</select>
	</label>

	<script>
	this.mixin(Injection);
	this.injectOnce('.item-select', '.select-triangle', $.fn.itemSelect);
	</script>
</configuration-dropdown>

<configuration-option>
	<option class="item-select-option">
		<yield />
	</option>
</configuration-option>
