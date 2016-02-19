var Attribute = {
	Attribute: function (defVal, onSet) {
		this.val = val;
		this.onSet = onSet;
		if(onSet) {
			onSet.bind(this)(val);
		}
	}
}

Attribute.Attribute.prototype = {
	set: function (val) {
		// "undefined" is not a valid value for any attribute I'm tracking
		if(val !== undefined) {
			this.val = val;
			if(this.onSet) {
				this.onSet(val);
			}
		}
	},

	get: function () {
		return this.val;
	}
}
