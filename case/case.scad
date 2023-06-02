use <mattwach/util.scad>
include <heat_sink.scad>

module case() {
  heat_sink();
}

$fa=2.0;
$fs=0.5;
case();
