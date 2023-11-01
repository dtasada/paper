import pylot
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

# Initialize aircraft dictionary
aircraft_dict = {
    "units" : "English",
    "CG" : [-0.11, 0, 0],
    "weight" : 30.0,
    "inertia" : {
        "Ixx" : 1.035,
        "Iyy" : 1.529,
        "Izz" : 8.411,
        "Ixy" : 0.0,
        "Ixz" : 0.013,
        "Iyz" : 0.0
    },
    "angular_momentum" : [0.0, 0.0, 0.0],
    "reference" : {
    },
    "controls" : {
        "aileron" : {
            "is_symmetric" : False,
            "max_deflection" : 20.0,
            "input_axis" : 0
        },
        "elevator" : {
            "is_symmetric" : True,
            "max_deflection" : 20.0,
            "input_axis" : 1
        },
        "rudder" : {
            "is_symmetric" : False,
            "max_deflection" : 20.0,
            "input_axis" : 2
        },
        "throttle" : {
            "input_axis" :3
        }
    },
    "engines" : {
        "engine1" : {
            "offset" : [0.0, 0.0, 0.0],
            "T0" : 30,
            "T1" : 0.0,
            "T2" : 0.0,
            "a" : 1.0,
            "control" : "throttle"
        }
    },
    "aero_model" : {
        "type" : "MachUpX",
        "solver" : "linear"
    },
    "airfoils" : {
        "NACA_0010" : {
            "type" : "linear",
            "aL0" : 0.0,
            "CLa" : 6.4336,
            "CmL0" : 0.0,
            "Cma" : 0.00,
            "CD0" : 0.00513,
            "CD1" : 0.0,
            "CD2" : 0.0984,
            "geometry" : {
                "NACA" : "0010"
            }
        }
    },
    "wings" : {
        "main_wing" : {
            "ID" : 1,
            "side" : "both",
            "is_main" : True,
            "semispan" : 4.0,
            "sweep" : 20.0,
            "dihedral" : [[0.0,0.0],
                          [0.8,5.0],
                          [1.0,80.0]],
            "chord" : [[0.0, 2.0],
                       [0.2, 1.0],
                       [1.0, 0.5]],
            "airfoil" : "NACA_0010",
            "control_surface" : {
                "chord_fraction" : 0.3,
                "root_span" : 0.45,
                "tip_span" : 0.95,
                "control_mixing" : {
                    "aileron" : 0.4,
                    "elevator" : 1.0
                }
            },
            "grid" : {
                "N" : 20,
                "flap_edge_cluster" : True
            }
        },
        "v_stab" : {
            "ID" : 2,
            "side" : "right",
            "is_main" : False,
            "connect_to" : {
                "ID" : 1,
                "dx" : -2.0,
                "dz" : 0.25,
                "location" : "root"
            },
            "semispan" : 1.0,
            "chord" : [[0.0, 0.1],
                       [0.5, 0.4],
                       [1.0, 0.1]],
            "dihedral" : 90.0,
            "aifoil" : "NACA_0010",
            "control_surface" : {
                "chord_fraction" : 0.2,
                "root_span" : 0.0,
                "tip_span" : 1.0,
                "control_mixing" : {
                    "rudder" : 1.0
                }
            },
            "grid" : {
                "N" : 10
            }
        }
    }
}

# Initialize simulation dictionary
sim_dict = {
    # Set simulation parameters here
    "tag" : "Example of flight simulator input.",
    "simulation" : {
        "real_time" : True,
        "enable_graphics" : True,
        "target_framerate" : 30
    },
    "units" : "English",
    "atmosphere" : {
        "density" : 0.0023769
    },
    "aircraft" : {
        "name" : "my_airplane",
        "file" : "examples/LinearizedModel/basic_high_wing.json",
        "trim" : {
            "airspeed" : 175,
            "position" : [0.0, 0.0, -500.0],
            "climb_angle" : 5.0,
            "bank_angle" : 20.0,
            "heading" : 0.0,
            "trim_controls" : ["throttle", "aileron", "elevator", "rudder"]
        },
        "controller" : "keyboard"
    }
}

# Run the sim
if __name__ == "__main__":
    sim = pylot.Simulator(sim_dict, verbose=True)
    sim.run_sim()
