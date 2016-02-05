<configuration-container>
	<div class='item-container'>
		<div class='item-container-header'>
			{ opts.title }
		</div>

		<yield />

		<div class='item-container-footer'>
			{ opts.footer }
		</div>
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

<configuration-checkbox>
	<label class='item'>
		<yield />
		<input id={ inputid } type='checkbox' class='item-toggle'></input>
	</label>
</configuration-checkbox>
