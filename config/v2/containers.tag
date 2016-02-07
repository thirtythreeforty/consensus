var Injection = {
	injectOnce: function(toFind, toExclude, func) {
		// Close over 'readied'
		var readied = false;
		this.one('mount', function() {
			if(!readied) {
				func.call($(this.root).find(toFind).not('*>'+toExclude));
			}
		});
		//$(function() { readied = true; console.log("ready called"); });
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
		<input type='checkbox' class='item-toggle' checked={ opts.attrib.get() } onclick={ onclickhandle }></input>
	</label>

	<script>
	this.mixin(Injection);
	this.injectOnce('.item-toggle', '.item-style-toggle', $.fn.itemToggle);

	onclickhandle(e) {
		opts.attrib.set(e.target.checked);
	}
	</script>
</configuration-toggle>

<configuration-dropdown>
	<label class="item">
		{ opts.caption }
		<select style="text-align:right;" class='item-select' name={ opts.selectname } onchange={ onchangehandle } value={ opts.attrib.get() }>
			<yield />
		</select>
	</label>

	<script>
	this.mixin(Injection);
	this.injectOnce('.item-select', '.select-triangle', $.fn.itemSelect);

	onchangehandle(e) {
		opts.attrib.set(e.target.value);
	}
	</script>
</configuration-dropdown>

<configuration-option>
	<option class="item-select-option">
		<yield />
	</option>
</configuration-option>
