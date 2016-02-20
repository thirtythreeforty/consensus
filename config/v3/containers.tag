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

<configuration-input>
	<label class='item'>
		<div class='item-input-wrapper'>
			<input type={ opts.input_type } placeholder={ opts.input_placeholder } class='item-input' value={ opts.attrib.get() } oninput={ oninputhandle }></input>
		</div>
	</label>

	<script>
	oninputhandle(e) {
		opts.attrib.set(e.target.value);
	}
	</script>
</configuration-input>

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

<configuration-radiobutton>
	<label class='item'>
		<yield />
		<input type='radio' class='item-radio' name={ opts.group } value={ opts.val } oncheck={ onchangehandle } checked={ opts.attrib.get() }>
	</label>

	<script>
	this.mixin(Injection);
	this.injectOnce('.item-radio', '.item-styled-radio', $.fn.itemRadio);

	onchangehandle(e) {
		opts.attrib.set(e.target.value);
	}
	</script>
</configuration-radiobutton>
