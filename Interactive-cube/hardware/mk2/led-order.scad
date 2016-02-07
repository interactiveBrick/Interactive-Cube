

module side($color, $label, $label2) {
  union() {
    // PLANE
    color($color) scale([10, 10, 1]) cube(center=true);

    // BUTTON GRID
	for (j = [0 : 3]) for (i = [0 : 3]) {
      translate([i*1.5-2.25,j*1.5-2.25,0.6]) {
          
      color("black")
      scale([1.0,1.0,0.2])
      cube(center=true);

    color("yellow") 
          translate([-0.4,-0.2,0])
      scale([0.05,0.05, 0.15])
        linear_extrude(height = 1.0) {
          text(str((3-j)*4+i), font="Liberation Sans:style=Bold Italic");
        }
      }
    }
   
      
    // ARROW
    color("yellow") translate([-2.5, 4.0, 0.5]) scale([0.5,0.5,1.0]) union() {
      translate([-1.4, 0.0, 0]) scale([5, 0.5, 0.3]) cube(center=true);
      translate([0.5, -.5, 0]) rotate([0, 0, 45]) scale([2, 0.5, 0.3]) cube(center=true);
	
      translate([0.5, 0.5, 0]) rotate([0,0,-45]) scale([2, 0.5, 0.3]) cube(center=true);



	}

    // TEXT 
    color("yellow")
	translate([-4.5,-4.5, 0.3])
      scale([0.15,0.15, 0.3])
        linear_extrude(height = 1.0) {
          text($label, font="Liberation Sans:style=Bold Italic");
        }

    // TEXT 2
    color("yellow")
	translate([3.0,-4.5, 0.3])
      scale([0.15,0.15, 0.3])
        linear_extrude(height = 1.0) {
          text($label2, font="Liberation Sans:style=Bold Italic");
        }
}}

// side("green", "TP");

translate(v=[0,0,5]) rotate([0,0,0]) side("red", "TP", "4");
translate(v=[0,0,-5]) rotate([0,180,180]) side("red", "BT", "5");
translate(v=[0,-5,0]) rotate([90,0,0]) side("cyan", "FT", "0");
translate(v=[0,5,0]) rotate([90,0,180]) side("cyan", "BK", "2");
translate(v=[5,0,0]) rotate([90,0,90]) side("green", "RT", "1");
translate(v=[-5,0,0]) rotate([90,0,-90]) side("green", "LT", "3");
