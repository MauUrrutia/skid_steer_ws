#!/usr/bin/env python3
# import rclpy 
# import rclpy.time
# from rclpy.node import Node
# from nav_msgs.msg import Path
# from geometry_msgs.msg import Twist, PoseStamped, Pose
# from tf2_ros import Buffer, TransformListener
# from tf_transformations import quaternion_matrix, concatenate_matrices, quaternion_from_matrix, translation_from_matrix, inverse_matrix
# import math
# class pure_pursuit(Node):
#     def __init__(self):
#         super().__init__("pure_pursuit")
#         self.declare_parameter("look_ahead_distance", 0.3)
#         self.declare_parameter("max_linear_velocity", 0.3)
#         self.declare_parameter("max_angular_velocity", 0.5)

#         self.look_ahead_distance = self.get_parameter("look_ahead_distance").value
#         self.max_linear_velocity = self.get_parameter("max_linear_velocity").value
#         self.max_angular_velocity = self.get_parameter("max_angular_velocity").value

#         self.tf_buffer = Buffer()
#         self.tf_listener = TransformListener(self.tf_buffer, self)
#         self.path_subscriber = self.create_subscription(Path, "/a_star/path", self.path_callback, 10)
#         self.cmd_vel_publisher = self.create_publisher(Twist, "/cmd_vel", 10)
#         self.check_point_publisher = self.create_publisher(PoseStamped, "/pure_pursuit/check_point", 10)

#         self.timer = self.create_timer(0.1, self.control_loop)
        
#         self.global_plan = None


#     def path_callback(self, path: Path):
#         self.global_plan = path

#     def control_loop(self):
#         if not self.global_plan or not self.global_plan.poses:
#             return
#         try:
#             robot_pose_transform = self.tf_buffer.lookup_transform("odom", "base_footprint", rclpy.time.Time())
#         except Exception as e:
#             self.get_logger().warn(f"Could not transform: {e}")
#             return
        
#         if not self.transform_plan(robot_pose_transform.header.frame_id):
#             self.get_logger().error(f"Unable to transform Plan in robot's frame: {robot_pose_transform.header.frame_id}")
#             return

#         robot_pose = PoseStamped()
#         robot_pose.header.frame_id = robot_pose_transform.header.frame_id
#         robot_pose.pose.position.x = robot_pose_transform.transform.translation.x
#         robot_pose.pose.position.y = robot_pose_transform.transform.translation.y
#         robot_pose.pose.position.z = robot_pose_transform.transform.translation.z

#         robot_pose.pose.orientation = robot_pose_transform.transform.rotation

#         check_point_pose: PoseStamped = self.get_chek_point_pose(robot_pose)
#         dx = check_point_pose.pose.position.x - robot_pose.pose.position.x
#         dy = check_point_pose.pose.position.y - robot_pose.pose.position.y
#         dz = check_point_pose.pose.position.z - robot_pose.pose.position.z
#         distance = math.sqrt(dx**2 + dy**2 + dz**2)
#         if distance <= 0.1:
#             self.get_logger().info("Goal reached")
#             self.global_plan.poses.clear()
#             return

#         self.check_point_publisher.publish(check_point_pose)
#         robot_tf = quaternion_matrix([
#             robot_pose.pose.orientation.x,
#             robot_pose.pose.orientation.y,
#             robot_pose.pose.orientation.z,
#             robot_pose.pose.orientation.w
#         ])
#         robot_tf[0][3] = robot_pose.pose.position.x
#         robot_tf[1][3] = robot_pose.pose.position.y
#         robot_tf[2][3] = robot_pose.pose.position.z

#         check_point_pose_tf = quaternion_matrix([
#             check_point_pose.pose.orientation.x,
#             check_point_pose.pose.orientation.y,
#             check_point_pose.pose.orientation.z,
#             check_point_pose.pose.orientation.w
#         ])
#         check_point_pose_tf[0][3] = check_point_pose.pose.position.x
#         check_point_pose_tf[1][3] = check_point_pose.pose.position.y
#         check_point_pose_tf[2][3] = check_point_pose.pose.position.z

#         check_point_pose_robot_tf = concatenate_matrices(inverse_matrix(robot_tf), check_point_pose_tf)
#         check_point_pose_robot = PoseStamped()
#         check_point_pose_robot.pose.position.x = check_point_pose_robot_tf[0][3]
#         check_point_pose_robot.pose.position.y = check_point_pose_robot_tf[1][3]
#         check_point_pose_robot.pose.position.z = check_point_pose_robot_tf[2][3]
#         q = quaternion_from_matrix(check_point_pose_robot_tf)
#         check_point_pose_robot.pose.orientation.x = q[0]
#         check_point_pose_robot.pose.orientation.y = q[1]
#         check_point_pose_robot.pose.orientation.z = q[2]
#         check_point_pose_robot.pose.orientation.w = q[3]

#         curvature = self.get_curvature(check_point_pose_robot.pose)
#         cmd_vel = Twist()
#         cmd_vel.linear.x = self.max_linear_velocity
#         cmd_vel.angular.z = curvature * self.max_angular_velocity

#         self.cmd_vel_publisher.publish(cmd_vel)

#     def transform_plan(self, frame: str)->bool:
#         if self.global_plan.header.frame_id == frame:
#             return True
        
#         try:
#             transform = self.tf_buffer.lookup_transform(frame, self.global_plan.header.frame_id, rclpy.time.Time())
#         except Exception as e:
#             self.get_logger().info(f"Couldn't transform plan from frame: {self.global_plan.header.frame_id} to {frame}: {e}")
#             return False

#         transform_matrix = quaternion_matrix([transform.transform.rotation.x,
#                                               transform.transform.rotation.y,
#                                               transform.transform.rotation.z,
#                                               transform.transform.rotation.w])
        
#         transform_matrix[0][3] = transform.transform.translation.x
#         transform_matrix[1][3] = transform.transform.translation.y
#         transform_matrix[2][3] = transform.transform.translation.z
        
#         for pose in self.global_plan.poses:
#             pose_matrix = quaternion_matrix([ pose.pose.orientation.x,
#                                               pose.pose.orientation.y,
#                                               pose.pose.orientation.z,
#                                               pose.pose.orientation.w])
        
#             pose_matrix[0][3] = pose.pose.position.x
#             pose_matrix[1][3] = pose.pose.position.y
#             pose_matrix[2][3] = pose.pose.position.z
#             transformed_pose = concatenate_matrices(pose_matrix, transform_matrix)
            
#             [pose.pose.orientation.x, pose.pose.orientation.y, 
#              pose.pose.orientation.z, pose.pose.orientation.w,] = quaternion_from_matrix(transformed_pose)
            
#             [pose.pose.position.x, pose.pose.position.y, pose.pose.position.z] = translation_from_matrix(transformed_pose)
#             pose.header.frame_id = frame

#         self.global_plan.header.frame_id = frame
#         return True

#     def get_chek_point_pose(self, robot_pose: PoseStamped):
#         check_point_pose = self.global_plan.poses[-1]
#         for pose in reversed(self.global_plan.poses):
#             dx = pose.pose.position.x - robot_pose.pose.position.x
#             dy = pose.pose.position.y - robot_pose.pose.position.y
#             dz = pose.pose.position.z - robot_pose.pose.position.z
#             distance = math.sqrt(dx**2 + dy**2 + dz**2)
#             if distance > self.look_ahead_distance:
#                 check_point_pose = pose
#             else:
#                 break
#         return check_point_pose

#     def get_curvature(self, check_point_pose: Pose):
#         L = check_point_pose.position.x**2 + check_point_pose.position.y**2
#         if L > 0.001:
#             return 2.0 * check_point_pose.position.y / L
#         else:
#             return 00.0

# def main():
#     rclpy.init()
#     node = pure_pursuit()
#     rclpy.spin(node)
#     node.destroy_node()
#     rclpy.shutdown()


# if __name__ == '__main__':
#     main()

#!/usr/bin/env python3
import rclpy 
import rclpy.time
from rclpy.node import Node
from nav_msgs.msg import Path
from geometry_msgs.msg import Twist, PoseStamped, Pose, Point
from tf2_ros import Buffer, TransformListener
from tf_transformations import euler_from_quaternion
import math

class pure_pursuit(Node):
    def __init__(self):
        super().__init__("pure_pursuit")
        self.declare_parameter("look_ahead_distance", 0.3)  # Aumentado para suavizar
        self.declare_parameter("max_linear_velocity", 0.3)
        self.declare_parameter("max_angular_velocity", 1.0)  # Reducido
        self.declare_parameter("goal_tolerance", 0.1)
        self.declare_parameter("min_linear_velocity", 0.05)  # Nueva: velocidad mínima

        self.look_ahead_distance = self.get_parameter("look_ahead_distance").value
        self.max_linear_velocity = self.get_parameter("max_linear_velocity").value
        self.max_angular_velocity = self.get_parameter("max_angular_velocity").value
        self.goal_tolerance = self.get_parameter("goal_tolerance").value
        self.min_linear_velocity = self.get_parameter("min_linear_velocity").value

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.path_subscriber = self.create_subscription(Path, "/a_star/path", self.path_callback, 10)
        self.cmd_vel_publisher = self.create_publisher(Twist, "/cmd_vel", 10)
        self.check_point_publisher = self.create_publisher(PoseStamped, "/pure_pursuit/check_point", 10)

        self.timer = self.create_timer(0.1, self.control_loop)
        
        self.global_plan = None
        self.current_goal_index = 0

    def path_callback(self, path: Path):
        self.global_plan = path
        self.current_goal_index = 0
        self.get_logger().info(f"New path received with {len(path.poses)} points")

    def control_loop(self):
        if not self.global_plan or not self.global_plan.poses:
            return
            
        try:
            # Obtener la pose del robot en el marco odom
            robot_pose_transform = self.tf_buffer.lookup_transform(
                "odom", "base_footprint", rclpy.time.Time()
            )
        except Exception as e:
            self.get_logger().warn(f"Could not transform: {e}")
            return

        # Crear PoseStamped del robot
        robot_pose = PoseStamped()
        robot_pose.header.frame_id = robot_pose_transform.header.frame_id
        robot_pose.pose.position.x = robot_pose_transform.transform.translation.x
        robot_pose.pose.position.y = robot_pose_transform.transform.translation.y
        robot_pose.pose.position.z = robot_pose_transform.transform.translation.z
        robot_pose.pose.orientation = robot_pose_transform.transform.rotation

        # Encontrar el punto de mira (lookahead point)
        lookahead_point, goal_reached = self.get_lookahead_point(robot_pose)
        
        if goal_reached:
            self.get_logger().info("Goal reached!")
            cmd_vel = Twist()  # Comando de velocidad cero
            self.cmd_vel_publisher.publish(cmd_vel)
            self.global_plan = None
            return

        # Publicar el punto de mira para visualización
        lookahead_pose = PoseStamped()
        lookahead_pose.header = robot_pose.header
        lookahead_pose.pose.position = lookahead_point
        self.check_point_publisher.publish(lookahead_pose)

        # Calcular la curvatura (forma simplificada)
        curvature = self.calculate_curvature(robot_pose, lookahead_point)
        
        # Calcular velocidades
        cmd_vel = self.calculate_velocities(curvature, robot_pose, lookahead_point)
        
        self.cmd_vel_publisher.publish(cmd_vel)

    def get_lookahead_point(self, robot_pose: PoseStamped):
        """Encuentra el punto de mira en el path"""
        closest_dist = float('inf')
        closest_idx = 0
        goal_reached = False
        
        # Encontrar el punto más cercano en el path
        for i, path_pose in enumerate(self.global_plan.poses):
            dx = path_pose.pose.position.x - robot_pose.pose.position.x
            dy = path_pose.pose.position.y - robot_pose.pose.position.y
            distance = math.sqrt(dx**2 + dy**2)
            
            if distance < closest_dist:
                closest_dist = distance
                closest_idx = i
        
        # Verificar si alcanzamos el goal
        goal_pose = self.global_plan.poses[-1]
        goal_dx = goal_pose.pose.position.x - robot_pose.pose.position.x
        goal_dy = goal_pose.pose.position.y - robot_pose.pose.position.y
        goal_distance = math.sqrt(goal_dx**2 + goal_dy**2)
        
        if goal_distance < self.goal_tolerance:
            return Point(), True
        
        # Buscar el punto de mira
        lookahead_point = Point()
        found_lookahead = False
        
        for i in range(closest_idx, len(self.global_plan.poses)):
            path_pose = self.global_plan.poses[i]
            dx = path_pose.pose.position.x - robot_pose.pose.position.x
            dy = path_pose.pose.position.y - robot_pose.pose.position.y
            distance = math.sqrt(dx**2 + dy**2)
            
            if distance >= self.look_ahead_distance:
                lookahead_point = path_pose.pose.position
                found_lookahead = True
                break
        
        # Si no encontramos punto de mira, usar el último punto
        if not found_lookahead:
            lookahead_point = self.global_plan.poses[-1].pose.position
        
        return lookahead_point, False

    def calculate_curvature(self, robot_pose: PoseStamped, lookahead_point: Point):
        """Calcula la curvatura usando la fórmula simplificada de Pure Pursuit"""
        # Obtener el yaw del robot
        orientation_q = robot_pose.pose.orientation
        orientation_list = [orientation_q.x, orientation_q.y, orientation_q.z, orientation_q.w]
        _, _, yaw = euler_from_quaternion(orientation_list)
        
        # Vector desde robot al punto de mira
        dx = lookahead_point.x - robot_pose.pose.position.x
        dy = lookahead_point.y - robot_pose.pose.position.y
        
        # Transformar a coordenadas del robot
        robot_dx = dx * math.cos(-yaw) - dy * math.sin(-yaw)
        robot_dy = dx * math.sin(-yaw) + dy * math.cos(-yaw)
        
        # Calcular curvatura (2*y / L^2)
        L_squared = robot_dx**2 + robot_dy**2
        if L_squared > 0.001:
            curvature = 2.0 * robot_dy / L_squared
        else:
            curvature = 0.0
            
        return curvature

    def calculate_velocities(self, curvature, robot_pose: PoseStamped, lookahead_point: Point):
        """Calcula las velocidades lineales y angulares con saturación"""
        cmd_vel = Twist()
        
        # Velocidad angular proporcional a la curvatura
        angular_vel = curvature * self.max_linear_velocity
        angular_vel = max(min(angular_vel, self.max_angular_velocity), -self.max_angular_velocity)
        
        # Velocidad lineal - reducir cuando hay curvas cerradas
        linear_vel = self.max_linear_velocity
        
        # Reducir velocidad en curvas cerradas
        if abs(curvature) > 0.5:
            linear_vel = self.max_linear_velocity * 0.5
        
        # Reducir velocidad cuando nos acercamos al goal
        goal_pose = self.global_plan.poses[-1]
        goal_dx = goal_pose.pose.position.x - robot_pose.pose.position.x
        goal_dy = goal_pose.pose.position.y - robot_pose.pose.position.y
        goal_distance = math.sqrt(goal_dx**2 + goal_dy**2)
        
        if goal_distance < 0.5:
            linear_vel = linear_vel * (goal_distance / 0.5)
            linear_vel = max(linear_vel, self.min_linear_velocity)
        
        # Asegurar que la velocidad lineal no sea demasiado baja para girar
        if abs(angular_vel) > 0.1 and linear_vel < self.min_linear_velocity * 2:
            linear_vel = self.min_linear_velocity * 2
        
        cmd_vel.linear.x = linear_vel
        cmd_vel.angular.z = angular_vel
        
        return cmd_vel

def main():
    rclpy.init()
    node = pure_pursuit()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()