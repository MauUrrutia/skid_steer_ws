import os
from launch import LaunchDescription
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node
from launch.substitutions import Command
from ament_index_python.packages import get_package_share_directory
def generate_launch_description():
    skid_steer_share_dir = get_package_share_directory('skid_steer_description')
    urdf_file = ParameterValue(Command([
                                    "xacro ", 
                                    os.path.join(skid_steer_share_dir, 'urdf', 'skid_steer_V2.urdf.xacro'),
                                    " is_sim:=False",]),
                                value_type=str)
    
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[
            {'robot_description': urdf_file}
        ]
    )

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            {"robot_description": urdf_file},
            {"use_sim_time": False},
            os.path.join(
                get_package_share_directory("skid_steer_controller"),
                "config",
                "skid_steer_controllers.yaml"
            )

        ]
    )
    
    
    return LaunchDescription([
        robot_state_publisher_node,
        controller_manager
    ])