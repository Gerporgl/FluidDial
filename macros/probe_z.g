#<fast_rate>=80
#<slow_rate>=20
#<probe_dist>=60
#<probe_offset>=19.33 ; puck height
#<retract_dist>=5

G91
G38.2 Z[-#<probe_dist>] F#<fast_rate> ; probe fast
G1 Z1 F1000 ; retract a little
G38.2 Z[-#<probe_dist>] F#<slow_rate> ; probe slowly
G90
G10 L20 P0 Z[#<probe_offset>]
$J=G91 G21 F1000 Z[#<retract_dist>]
