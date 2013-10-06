describe("Jasmine Drupal Integration module testing", function() {

	beforeEach(function() {
	});

	it("Testing if Jasmine Drupal Integration object is there", function() { 
		expect(jasmine.version_.build).toEqual(1);
	});

	afterEach(function () {
	});

});