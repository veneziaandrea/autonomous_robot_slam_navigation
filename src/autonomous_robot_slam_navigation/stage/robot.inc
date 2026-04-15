define sick ranger 
(
  sensor
  (
    pose [ 0.0 0.0 0.0 0.0 ]
    size [0.1 0.1 0.1 ]
    range_max 100.0
    fov 360.0
    samples 1242
  )
  # generic model properties
  color "red"
  size [ 0.06 0.15 0.03 ]
)

define my_robot position
(
  pose [ 0.0 0.0 0.0 0.0 ]

  localization "odom"

  odom_error [0.01 0.01 0.01 0.01]

  size [ 0.54 0.40 0.20 ]
  origin [ 0.0 0.0 0.0 0.0 ]
  gui_nose 1
  drive "diff"
  color "grey"
  
  sick()
)
