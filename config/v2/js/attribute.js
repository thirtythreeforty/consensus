var Attribute = {
	Attribute: function (defVal) {
		this.var = defVal;
	}
}

Attribute.Attribute.prototype = {
	set: function (val) {
		this.var = val;
		if(this.onSet) {
			this.onSet(val);
		}
	},

	get: function () {
		return this.var;
	}
}
