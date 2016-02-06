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
	// Slate has already run its injection, so we need to ask for it again.
	// Fortunately (?), they add their functions to the global jQuery object.
	this.one('mount', function() { $(this.root).find('.item-toggle').itemToggle(); });
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
	this.one('mount', function() { $(this.root).find('.item-select').itemSelect(); });
	</script>
</configuration-dropdown>

<configuration-option>
	<option class="item-select-option">
		<yield />
	</option>
</configuration-option>
