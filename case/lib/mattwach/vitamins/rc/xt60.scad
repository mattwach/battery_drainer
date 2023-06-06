module xt60() {
  width = 15.7;
  length = 16;
  height = 8.1;
  shell_offset = 0.65;
  overlap = 0.1;

  module base() {
    chamfer = 3.1;

    polygon([
        [0, 0],  // bottom corner
        [width - chamfer, 0],  // bottom edge
        [width, chamfer],
        [width,   height - chamfer],  // right side
        [width - chamfer, height],
        [0,    height],
        [0,      0]
    ]);
  }

  module pin_hole(x_pos) {
      translate([x_pos, height / 2, -1]) cylinder(h=4, d=6.3);
      translate([x_pos, height / 2, -1]) cylinder(h=8, d=4.3);
  }

  module guide_tab() {
    tab_width = 1.5;
    tab_height = 1;
    tab_depth = 8;
    translate([(width - tab_width) / 2, shell_offset - overlap, 6 - overlap])
      cube([tab_width, 1 + overlap, tab_depth + overlap]); 
  }

  module pin() {
    base_length = 11.4;
    pin_diameter = 3.6;
    pin_height = 8;

    module pin_top() {
      module pin_cutout() {
        cube([pin_diameter, 0.6, pin_height * 2], center=true); 
      }

      translate([0, 0, base_length])
        difference() {
          minkowski() {
            cylinder(d=pin_diameter - 2, h=pin_height - 1);
            sphere(r=1);
          }
          pin_cutout();
          rotate([0, 0, 90]) pin_cutout();
          cylinder(d=1.7, h=pin_height);
        }
    }

    module pin_bottom() {
      bottom_diameter = 4.3;
      difference() {
        cylinder(d=bottom_diameter, h=base_length);
        translate([0, 0, -overlap]) cylinder(d=3.1, h=8 + overlap);
        translate([-bottom_diameter / 2, 0, -overlap])
          cube([bottom_diameter, bottom_diameter, 3.5 + overlap]);
      }
    }

    color("gold") {
      pin_bottom();
      pin_top();
    }
  }

  pinx = [4.35, 11.35];

  color("yellow", 0.8) {
    difference() {
      linear_extrude(length) base();
      translate([0,0,6]) linear_extrude(length) offset(-shell_offset) base();
      for (i = [0:1]) {
        pin_hole(pinx[i]);
      }
    }
    guide_tab();
  }

  for (i = [0:1]) {
    translate([pinx[i], height/2, -5]) rotate([0, 0, 90 + 180 * i]) pin();
  }
}

module xt60_pw_male() {
  overlap = 0.01;
  xt60_pw_length = 18.4;
  xt60_pw_width = 15.6;
  xt60_pw_height = 8.1;
  xt60_pin_yoffset = 4.2;
  xt60_pin_yspan = 6.8;

  extrude_depth = 8;

  module shell() {
    module base() {
      back_fillet = 4;
      hull() {
        cube([1, xt60_pw_width, 1]);
        tz(xt60_pw_height - back_fillet) tx(back_fillet) rx(-90) cylinder(r=back_fillet, h=xt60_pw_width);
        tx(xt60_pw_length - 1) cube([1, xt60_pw_width, xt60_pw_height]);
      }
    }

    base();
  }

  module inner() {
    shell = 0.7;
    slope = 2.5;
    peg_width = 1.5;
    peg_height = 1;
    width = xt60_pw_width - shell * 2;
    center = (width - slope) / 2;
    height = xt60_pw_height - shell * 2;
    tx(xt60_pw_length - extrude_depth) rz(90) rx(90) linear_extrude(extrude_depth + overlap)
      polygon([
          [shell, shell],
          [shell + center - peg_width / 2, shell],
          [shell + center - peg_width / 2, shell + peg_height],
          [shell + center + peg_width / 2, shell + peg_height],
          [shell + center + peg_width / 2, shell],
          [shell + width - slope, shell],
          [shell + width, shell + slope],
          [shell + width, shell + height - slope],
          [shell + width - slope, shell + height],
          [shell, shell + height],
      ]);
  }

  module inner_pins() {
    module pin() {
      pin_diameter = 3.7;
      pin_inset = 1.5;
      module slot() {
        slot_width = 0.75;
        slot_depth = extrude_depth - 2;
        translate([
            -slot_width / 2,
            -pin_diameter / 2 - overlap,
            extrude_depth - slot_depth]) cube([
              slot_width,
              pin_diameter + overlap * 2,
              slot_depth + overlap]);
      }
      color("gold") translate([
          xt60_pw_length - extrude_depth - pin_inset,
          xt60_pin_yoffset,
          xt60_pw_height / 2]) ry(90) difference() {
        cylinder(d=pin_diameter, h=extrude_depth);
        slot();
        rz(90) slot();
      }
    }

    pin(); 
    ty(xt60_pin_yspan) pin(); 
  }

  module bottom_pins() {
    bottom_pin_diameter = 2.5;
    bottom_pin_height = 2.6;
    bottom_pin_xpad = 2.2;
    module pin() {
      color("gold") translate([
          bottom_pin_xpad,
          xt60_pin_yoffset,
          -bottom_pin_height]) cylinder(d=bottom_pin_diameter, h=bottom_pin_height + overlap);
    }

    pin();
    ty(xt60_pin_yspan) pin(); 
  }

  module support_pins() {
    support_pin_width = 1.6;
    support_pin_height = 3.2;
    support_pin_thickness = 0.5;
    support_pin_xpad = 7.2;
    support_pin_inset = 0.5;

    module bar() {
      cube([
          support_pin_width,
          support_pin_thickness,
          support_pin_height]);
    }

    module slot() {
      slot_thickness = 0.4;
      translate([
          (support_pin_width - slot_thickness) / 2,
          -overlap,
          -overlap]) cube([
            slot_thickness,
            support_pin_thickness + overlap * 2,
            support_pin_height]);
    }

    module pin() {
      translate([
          support_pin_xpad,
          support_pin_inset,
          -support_pin_height]) difference() {
        bar();
        slot();
      }
    }

    pin();
    ty(xt60_pw_width - support_pin_inset * 2 - support_pin_thickness) pin();
  }

  color("yellow") difference() {
    shell();
    inner();
  }
  inner_pins();
  bottom_pins();
  support_pins();
}

// $fn=36;
//xt60();
