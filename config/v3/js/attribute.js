var Attribute = {
	Attribute: function (defVal, onSet) {
		this.var = val;
		this.onSet = onSet;
		if(this.onSet) {
			this.onSet(val);
		}
	}
}

Attribute.Attribute.prototype = {
	set: function (val) {
		// "undefined" is not a valid value for any attribute I'm tracking
		if(val !== undefined) {
			this.var = val;
			if(this.onSet) {
				this.onSet(val);
			}
		}
	},

	get: function () {
		return this.var;
	}
}
