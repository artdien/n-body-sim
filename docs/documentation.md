# Introduction

This project implements an n-body simulation for bodies interacting via gravitational force. It provides an integrator for solving the n-body problem numerically in C++, implemented for both the CPU and the GPU. On the CPU, the integrator is multi-threaded and further accelerated using the Barnes-Hut algorithm. On the GPU, the integrator is implemented via compute shaders in OpenGL 'naively' using the all-pairs algorithm. The visualization of all bodies is performed in real-time by rendering them as circles using OpenGL.

The goal of this project was not to implement the most performant or exhaustive n-body simulation available, but rather to experiment with features introduced in more recent versions of C++ and OpenGL. Examples include ranges and concepts in C++ as well as direct state access (DSA) and persistent mapping in OpenGL. Consequently, these features are sometimes used in situations where they might seem 'too much'.

This document contains two main sections:

* [Theoretical foundation](#theoretical-foundation): This section contains the necessary theoretical foundation to understand the implementation, ranging from the physics of the n-body problem to the mathematical tools used to solve it.
* [Implementation](#implementation): This section describes the architecture of the implementation and the reasoning behind specific design decisions.

## Theoretical Foundation

### Notation

To state the n-body problem concisely, we first define our notation:

* Vectors or vector-valued functions are denoted in **boldface**.
* Quantities (e.g. positions) of body $i$ are denoted with a subscript $i$, where $i = 1, \dots, n$.
* Time is measured in seconds $[\text{s}]$ and is denoted by $t \in \mathbb{R}_{\ge 0}$.
* The position of body $i$ is measured in meters $[\text{m}]$ and is denoted by:
  $$\mathbf{x}_i \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3, \quad t \mapsto \mathbf{x}_i(t)$$
* The velocity of body $i$ is measured in meters per second $[\frac{\text{m}}{\text{s}}]$ and is denoted by:
  $$\mathbf{v}_i \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3, \quad t \mapsto \mathbf{v}_i(t)$$
* The acceleration of body $i$ is measured in meters per second squared $[\frac{\text{m}}{\text{s}^2}]$ and is denoted by:
  $$\mathbf{a}_i \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3, \quad t \mapsto \mathbf{a}_i(t)$$
* The mass of body $i$ is measured in kilograms $[\text{kg}]$ and is denoted by $m_i \in \mathbb{R}_{>0}$.
* The gravitational constant $6.6743 \cdot 10^{-11} \frac{\text{m}^3}{\text{kg}\,\text{s}^2}$ is denoted by $G$.
* The norm $\Vert \cdot \Vert \colon \mathbb{R}^3 \to \mathbb{R}, \quad \mathbf{x} \mapsto \Vert \mathbf{x} \Vert$ always denotes the Euclidean norm, i.e., $\Vert \mathbf{x} \Vert  = \sqrt{x^2 + y^2 + z^2}$.

### N-Body Problem

The n-body problem accounts for a total of $n$ bodies. While these bodies can interact via various forces, this project restricts the interaction to gravitational force, assuming each body $i$ is a point mass (having mass $m_i$ but no spatial size).

The gravitational force $\mathbf{f}_{ij}(t)$ exerted on body $i$ by body $j$ at time $t$ is defined as:

$$
\mathbf{f}_{ij} \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3,
\quad t \mapsto \mathbf{f}_{ij}(t)
= \frac{G m_i m_j}{\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert^2} \frac{(\mathbf{x}_j(t) - \mathbf{x}_i(t))}{\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert}
= \frac{G m_i m_j (\mathbf{x}_j(t) - \mathbf{x}_i(t))}{\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert^3}
$$

The magnitude of the gravitational force $\mathbf{f}_{ij}$ exerted on body $i$ follows an inverse square law, and its direction points towards body $j$. This makes gravity an attractive force.

In this simulation, we are interested in the total gravitational force exerted on each body. The total gravitational force $\mathbf{F}_i(t)$ exerted on body $i$ at time $t$ is the sum of all mutual gravitational forces $\mathbf{f}_{ij}$:

$$
\mathbf{F}_i \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3,
\quad t \mapsto \mathbf{F}_i(t)
= \sum_{j = 1, j \neq i}^n \frac{G m_i m_j (\mathbf{x}_j(t) - \mathbf{x}_i(t))}{\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert^3}
$$

The interaction of body $i$ with itself is excluded from the sum.

With the total gravitational force defined, we can formulate the n-body problem as finding the position $\mathbf{x}_i(t)$ for all bodies $i$ at all times $t$. To do this, we solve a set of ordinary differential equations (ODEs). Since velocity is the derivative of position and acceleration is the derivative of velocity, we obtain the following ODEs:

$$
\begin{aligned}
  \frac{\mathrm{d} \mathbf{x}_i(t)}{\mathrm{d}t} &= \mathbf{v}_i(t) \\
  \frac{\mathrm{d} \mathbf{v}_i(t)}{\mathrm{d}t} &= \mathbf{a}_i(t)
\end{aligned}
\quad \text{for} \quad i = 1, \dots, n
$$

This gives us $2n$ ODEs. To find a specific solution, we treat these as an initial value problem (IVP) with initial positions $\mathbf{x}_{i,0}$ and velocities $\mathbf{v}_{i,0}$ at time $t=0$. 

Using Newton's second law $\mathbf{F}_{i}(t) = m_i \mathbf{a}_i(t)$ with initial positions $\mathbf{x}_{i,0}$ and velocities $\mathbf{v}_{i,0}$, we express the n-body problem as the following IVP:

$$
\begin{aligned}
  \frac{\mathrm{d} \mathbf{x}_i(t)}{\mathrm{d}t} &= \mathbf{v}_i(t) \\
  \frac{\mathrm{d} \mathbf{v}_i(t)}{\mathrm{d}t} &= \frac{\mathbf{F}_i(t)}{m_i}
\end{aligned}
\quad \text{with} \quad
\begin{aligned}
  \mathbf{x}_i(0) &= \mathbf{x}_{i,0} \\
  \mathbf{v}_i(0) &= \mathbf{v}_{i,0}
\end{aligned}
\quad \text{for} \quad i = 1, \dots, n
$$

More information about the n-body problem can be found in \[1\].

### Softening factor

Looking at the gravitational force $\mathbf{F}_i(t)$, we notice that the function is undefined if the denominator is zero. This occurs if another body $j$ occupies the exact same position as body $i$. While unlikely in a real physical system, this can occur in simulations due to numerical errors. Even when positions are not identical, being sufficiently close leads to extreme force values that can make the simulation unstable.

To avoid this, we add a softening factor $\varepsilon \in \mathbb{R}_{>0}$ to the denominator to ensure it remains positive. The softening factor acts as a buffer between bodies and is typically very small to avoid significantly altering the physics. The updated gravitational force becomes:

$$
\mathbf{F}_i \colon \mathbb{R}_{\geq 0} \to \mathbb{R}^3,
\quad t \mapsto \mathbf{F}_i(t)
= \sum_{j = 1}^n \frac{G m_i m_j (\mathbf{x}_j(t) - \mathbf{x}_i(t))}{(\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert^2 + \varepsilon^2)^\frac{3}{2}}
$$

Since $\Vert \mathbf{x}_j(t) - \mathbf{x}_i(t) \Vert^2 \geq 0$ and $\varepsilon^2 > 0$, the denominator can never be zero. Squaring $\varepsilon$ ensures it has the same physical dimension as the squared distance.

A beneficial side effect is that we no longer need to explicitly exclude the interaction of a body with itself. In such cases, the numerator becomes zero, contributing nothing to the sum.

### Centered Mass and Velocity

It is often useful to set the position and velocity of the center of mass (COM) to zero to simplify simulations and prevent the entire system from drifting out of view. First, we calculate the COM position $\mathbf{x}_{\text{com}}(t)$ and velocity $\mathbf{v}_{\text{com}}(t)$:

$$
\begin{aligned}
  \mathbf{x}_{\text{com}}(t) &= \frac{1}{M} \sum_{i=1}^n m_i \mathbf{x}_i(t) \\
  \mathbf{v}_{\text{com}}(t) &= \frac{1}{M} \sum_{i=1}^n m_i \mathbf{v}_i(t)
\end{aligned}
\quad \text{with} \quad M = \sum_{i=1}^n m_i
$$

We then subtract these values from the positions and velocities of all bodies:

$$
\begin{aligned}
  \mathbf{x}'_i(t) &= \mathbf{x}_i(t) - \mathbf{x}_{\text{com}}(t) \\
  \mathbf{v}'_i(t) &= \mathbf{v}_i(t) - \mathbf{v}_{\text{com}}(t)
\end{aligned}
\quad \text{for} \quad i = 1, \dots, n
$$

The resulting $\mathbf{x}'_i(t)$ and $\mathbf{v}'_i(t)$ ensure the center of mass remains stationary at the origin with zero velocity.

### Integrators

To implement the simulation, we solve the n-body IVP numerically by discretizing the differential operators of the ODEs and deriving algorithms based on these discretizations. This procedure is mathematically akin to integration, which is why these algorithms are called integrators.

We start by discretizing time using a fixed time step $\Delta t > 0$:

$$
t_j = j \Delta t \quad \text{for} \quad j = 0, 1, 2, \dots
$$

Using forward finite differences for the differential operators, the ODEs can be discretized as:

$$
\begin{aligned}
  \frac{\mathbf{x}_i(t_{j+1}) - \mathbf{x}_i(t_j)}{\Delta t} + \mathcal{O}(\Delta t) &= \mathbf{v}_i(t_j)
  \quad &\Rightarrow \quad
  \mathbf{x}_i(t_{j+1}) &= \mathbf{x}_i(t_j) + \mathbf{v}_i(t_j) \Delta t  + \mathcal{O}(\Delta t) \\

  \frac{\mathbf{v}_i(t_{j+1}) - \mathbf{v}_i(t_j)}{\Delta t} + \mathcal{O}(\Delta t) &= \frac{\mathbf{F}_i(t_j)}{m_i}
  \quad &\Rightarrow \quad
  \mathbf{v}_i(t_{j+1}) &= \mathbf{v}_i(t_j) + \frac{\mathbf{F}_i(t_j)}{m_i} \Delta t + \mathcal{O}(\Delta t)
\end{aligned}
\quad \text{for} \quad j = 0, 1, 2, \dots
$$

This leads to the following algorithm for solving the n-body problem:

1. Calculate $\mathbf{F}_i(t_j)$.
2. Calculate $\mathbf{a}_i(t_j) = \frac{\mathbf{F}_i(t_j)}{m_i}$.
3. Update velocities $\mathbf{v}_i(t_{j+1}) = \mathbf{v}_i(t_j) + \mathbf{a}_i(t_j) \Delta t$.
4. Update positions $\mathbf{x}_i(t_{j+1}) = \mathbf{x}_i(t_j) + \mathbf{v}_i(t_j) \Delta t$.
5. Increment $t_j$ and repeat.

Each step is done for all bodies $i = 1, \dots, n$. Starting with initial values $\mathbf{x}_{i,0}$ and $\mathbf{v}_{i,0}$, we can thus iteratively calculate the positions and velocities at all times $t_j$. However, this integrator has two major problems:

* **Numerical Error:** The numerical error is proportional to $\mathcal{O}(\Delta t)$, requiring extremely small steps for accuracy.
* **Energy Conservation:** The integrator does not conserve energy. Instead, it adds energy to the system over time, eventually causing a "blow-up" where bodies are flung apart at unrealistic speeds. This is inherent to the integrator and not caused by insufficient numerical precision.

To mitigate the energy conservation problem, we can make the above integrator symplectic by replacing $\mathbf{v}_i(t_j)$ in step 4 with $\mathbf{v}_i(t_{j+1})$. A symplectic integrator better preserves the energy of the system and is therefore more suitable for long-time simulations. 

However, this does not change the numerical error. To improve it, we chose a different symplectic integrator: the velocity Verlet integrator, which has an error proportional to $\mathcal{O}(\Delta t^2)$. More information about the velocity Verlet integrator can be found in \[2\], here we will simply state the algorithm:

1. Calculate half-step velocities $\mathbf{v}_i(t_{j+0.5}) = \mathbf{v}_i(t_j) + \frac{1}{2} \mathbf{a}_i(t_j) \Delta t$.
2. Update positions $\mathbf{x}_i(t_{j+1}) = \mathbf{x}_i(t_j) + \mathbf{v}_i(t_{j+0.5}) \Delta t$.
3. Calculate forces $\mathbf{F}_i(t_{j+1})$ for all $i = 1, \dots, n$ using the updated positions.
4. Calculate accelerations $\mathbf{a}_i(t_{j+1}) = \frac{\mathbf{F}_i(t_{j+1})}{m_i}$.
5. Update velocities $\mathbf{v}_i(t_{j+1}) = \mathbf{v}_i(t_{j+0.5}) + \frac{1}{2} \mathbf{a}_i(t_{j+1}) \Delta t$ to full step.
6. Increment $t_j$ and repeat.

As above, each step is done for all bodies $i = 1, \dots, n$. Note that for the very first step ($t_0$), $\mathbf{a}_i(t_0)$ must be pre-calculated using the initial positions $\mathbf{x}_{i,0}$ before starting the loop. Alternatively, we can start with $\mathbf{a}_i(t_0) = \mathbf{0}$, making the simulation less precise but still acceptable. Lastly, the half-step velocity in the first step is an intermediate value used only to ensure higher-order accuracy and does not require other values at $t_{j+0.5}$ for its calculation.

### Barnes-Hut Algorithm

The integrators from the previous section require computing $\mathbf{F}_i(t)$ for every body in each time step. This is the primary computational bottleneck of an n-body simulation. The 'naive' all-pairs algorithm calculates forces by simply using the force formula as-is, which results in a runtime complexity of $\mathcal{O}(n^2)$, since there are $n$ forces to calculate and each single force calculation requires a summation over $n$ bodies.

The Barnes-Hut algorithm reduces this complexity to $\mathcal{O}(n \log n)$ by trading some numerical precision for performance. The core idea is to group distant bodies into single aggregate masses. If a group of bodies is sufficiently far away, their collective gravitational influence can be approximated using their total mass and center of mass. This can significantly reduce the number of summations when calculating forces.

These groups of bodies are managed via a spatial data structure called an octree. An octree is a tree-like data structure where each node represents a three-dimensional cuboid, which is recursively subdivided into eight smaller cuboids until each leaf node contains either zero or one body.

The Barnes-Hut algorithm consists of two phases:

1. **Constructing the octree:** Performed once per time step. The tree is built recursively, and the total mass and center of mass for every node are calculated during subdivision.
2. **Calculating forces:** For each body $i$, the tree is traversed recursively. At each node, we calculate $\theta = \frac{d}{r}$, where $d$ is the width of the cuboid (i.e. the width of its largest side) and $r$ is the distance to its center of mass. If $\theta$ is below a specific threshold, the cuboid's aggregate mass is used for the calculation and the traversal stops. Otherwise, the traversal continues deeper into the tree until the threshold is satisfied or the node only contains a single body.

Assuming a reasonable spatial distribution of bodies and value for the threshold $\theta$, traversing the octree takes $\mathcal{O}(\log n)$ per body, resulting in an overall runtime complexity of $\mathcal{O}(n \log n)$ for all bodies. The construction of the octree depends on the spatial distribution as well, but can be assumed to be $\mathcal{O}(n \log n)$ if the distribution is reasonable. Therefore, the Barnes-Hut algorithm can significantly reduce the runtime complexity compared to the all-pairs algorithm. For more information about the Barnes-Hut algorithm including pseudo-code, see \[3\].

### Special Configurations

In a general $n$-body simulation, initializing bodies with random positions and velocities typically results in an 'unstable' system. Because gravitational forces are highly sensitive to close encounters, such simulations often lead to 'slingshot' effects. Consequently, we consider the following specific configurations, i.e. initial positions and velocities, that are sufficiently stable to allow for long-term simulations.

In the following sections, we denote the gravitational force on a body $i$ as $\mathbf{F}^{\mathrm{G}}_i$ to distinguish it from other forces used in the derivations.

#### Euler's Collinear Configuration

Euler's collinear configuration is a three-body problem where all three bodies remain collinear while two of them orbit the third in circular motion. In this setup, all three bodies are assumed to have the same mass $m$.

The third body is positioned at the origin and remains stationary, i.e. $\mathbf{x}_3(0) = \mathbf{0}$ and $\mathbf{v}_3(0)=\mathbf{0}$. The other two bodies are placed at a distance $r \in \mathbb{R}_{>0}$ from the origin, i.e. $\mathbf{x}_1(0) = (r, 0, 0)$ and $\mathbf{x}_2(0) = (-r, 0, 0)$. For these bodies to maintain circular orbits, their initial velocities must take the form $\mathbf{v}_1(0) = \Vert \mathbf{v}_1(0) \Vert (0, 1, 0)$ and $\mathbf{v}_2(0) = \Vert \mathbf{v}_2(0) \Vert (0, -1, 0)$, where the magnitudes must be determined.

For bodies 1 and 2 to maintain circular motion, the centripetal force $\mathbf{F}^{\mathrm{C}}_i$ exerted on them must equal the gravitational force $\mathbf{F}^{\mathrm{G}}_i$. It is sufficient to set these forces equal at $t=0$. For $i \in \{1, 2\}$, the centripetal force $\mathbf{F}^{\mathrm{C}}_i(0)$ is:

$$
\mathbf{F}^{\mathrm{C}}_i(0)
= \frac{m \Vert \mathbf{v}_i(0) \Vert^2}{\Vert \mathbf{x}_3(0) - \mathbf{x}_i(0) \Vert} \frac{\mathbf{x}_3(0) - \mathbf{x}_i(0)}{\Vert \mathbf{x}_3(0) - \mathbf{x}_i(0) \Vert}
= -\frac{m \Vert \mathbf{v}_i(0) \Vert^2}{r^2} \mathbf{x}_i(0)
$$

Setting the two forces equal gives:

$$
\mathbf{F}^{\mathrm{C}}_i(0) = \mathbf{F}^{\mathrm{G}}_i(0)
\quad \Rightarrow \quad
-\frac{m \Vert \mathbf{v}_i(0) \Vert^2}{r^2} \mathbf{x}_i(0) = \sum_{k=1}^2 \frac{Gm^2 (\mathbf{x}_{j_k}(0) - \mathbf{x}_i(0))}{\Vert \mathbf{x}_{j_k}(0) - \mathbf{x}_i(0) \Vert^3}
\quad \text{with} \quad
j_k =
\begin{cases}
  j_1 = 2, j_2 = 3 &\text{for } &i = 1 \\
  j_1 = 1, j_2 = 3 &\text{for } &i = 2
\end{cases}
\quad \text{for} \quad i \in \{1, 2\}
$$

To simplify this equation, we first calculate the distance vectors:

$$
\begin{aligned}
  \mathbf{x}_{j_1}(0) - \mathbf{x}_i(0) &= -2\mathbf{x}_i(0) \\
  \mathbf{x}_{j_2}(0) - \mathbf{x}_i(0) &= -\mathbf{x}_i(0)
\end{aligned}
\quad \text{for} \quad i \in \{1, 2\}
$$

Substituting these into the previous equation:

$$
-\frac{m \Vert \mathbf{v}_i(0) \Vert^2}{r^2} \mathbf{x}_i(0)
= -\frac{Gm^2}{4 \Vert \mathbf{x}_i(0) \Vert^3} \mathbf{x}_i(0) - \frac{Gm^2}{\Vert \mathbf{x}_i(0) \Vert^3} \mathbf{x}_i(0)
= - \frac{5}{4}\frac{Gm^2}{r^3} \mathbf{x}_i(0)
\quad \text{for} \quad i \in \{1, 2\}
$$

Equating the coefficients of $\mathbf{x}_i(0)$ and solving for $\Vert \mathbf{v}_i(0) \Vert$ yields:

$$
\Vert \mathbf{v}_i(0) \Vert = \sqrt{\frac{5}{4} \frac{Gm}{r}} \quad \text{for} \quad i \in \{1, 2\}
$$

#### Lagrange's Periodic Configuration

Lagrange's periodic configuration is a three-body problem where the bodies form the vertices of an equilateral triangle and move in circular orbits. All three bodies are assumed to have the same mass $m$.

The first body is placed at position $\mathbf{x}_1(0) = (0, r, 0)$, where $r \in \mathbb{R}_{>0}$. To position the remaining two bodies such that they form an equilateral triangle, we apply a rotation matrix:

$$
R(\varphi) =
\begin{pmatrix}
  \cos(\varphi) & -\sin(\varphi) & 0 \\
  \sin(\varphi) &  \cos(\varphi) & 0 \\
  0 & 0 & 1
\end{pmatrix}
$$

This yields $\mathbf{x}_2(0) = R(\frac{2}{3} \pi)\,\mathbf{x}_1(0) = (-\frac{\sqrt{3}}{2}r, -\frac{1}{2}r, 0)$ and $\mathbf{x}_3(0) = R(\frac{2}{3} \pi)\,\mathbf{x}_2(0) = (\frac{\sqrt{3}}{2}r, -\frac{1}{2}r, 0)$. For body 1 to move in a circular orbit, its initial velocity must be of the form $\mathbf{v}_1(0) = \Vert \mathbf{v}_1(0) \Vert (-1, 0, 0)$. The velocities for bodies 2 and 3 are then derived using the same rotation matrix, i.e. $\mathbf{v}_2(0) = R(\frac{2}{3} \pi)\,\mathbf{v}_1(0)$ and $\mathbf{v}_3(0) = R(\frac{2}{3} \pi)\,\mathbf{v}_2(0)$.

Following a derivation similar to the collinear case, we set the gravitational force $\mathbf{F}^{\mathrm{G}}_1$ equal to the centripetal force $\mathbf{F}^{\mathrm{C}}_1$:

$$
\mathbf{F}^{\mathrm{C}}_1(0) = \mathbf{F}^{\mathrm{G}}_1(0)
\quad \Rightarrow \quad
-\frac{m \Vert \mathbf{v}_1(0) \Vert^2}{r^2} \mathbf{x}_1(0)
= \frac{Gm^2 (\mathbf{x}_2(0) - \mathbf{x}_1(0))}{\Vert \mathbf{x}_2(0) - \mathbf{x}_1(0) \Vert^3} + \frac{Gm^2 (\mathbf{x}_3(0) - \mathbf{x}_1(0))}{\Vert \mathbf{x}_3(0) - \mathbf{x}_1(0) \Vert^3}
$$

Calculating the relative positions to the first body:

$$
\begin{aligned}
  \mathbf{x}_2(0) - \mathbf{x}_1(0) &= \left(-\frac{\sqrt{3}}{2}r, -\frac{3}{2}r, 0 \right) \\
  \mathbf{x}_3(0) - \mathbf{x}_1(0) &= \left( \frac{\sqrt{3}}{2}r, -\frac{3}{2}r, 0 \right)
\end{aligned}
\quad \text{with} \quad
\Vert \mathbf{x}_2(0) - \mathbf{x}_1(0) \Vert
= \Vert \mathbf{x}_3(0) - \mathbf{x}_1(0) \Vert
= \sqrt{3}r
$$

Substituting these into the previous equation:

$$
-\frac{m \Vert \mathbf{v}_1(0) \Vert^2}{r^2} \mathbf{x}_1(0)
= \frac{Gm^2 (\mathbf{x}_2(0) - \mathbf{x}_1(0))}{3\sqrt{3}r^3} + \frac{Gm^2 (\mathbf{x}_3(0) - \mathbf{x}_1(0))}{3\sqrt{3}r^3}
$$

Since $\mathbf{x}_1(0)$ is non-zero only in its second component, we equate the scalars for that specific component:

$$
-\frac{m \Vert \mathbf{v}_1(0) \Vert^2}{r^2} r
= -\frac{3}{2}r \frac{Gm^2}{3\sqrt{3}r^3} - \frac{3}{2}r \frac{Gm^2}{3\sqrt{3}r^3}
= -\frac{Gm^2}{\sqrt{3}r^2}
$$

Solving for $\Vert \mathbf{v}_1(0) \Vert$ yields:

$$
\Vert \mathbf{v}_1(0) \Vert = \sqrt{\frac{Gm}{\sqrt{3}r}}
$$

#### Plummer Model

The Plummer model describes an $n$-body system where bodies follow a spherical density distribution. While not completely realistic, it serves as an excellent toy model for simulating star clusters. Because the model is based on a continuous distribution, initial positions and velocities are sampled rather than fixed.

The isotropic density prescribed by the Plummer model in spherical coordinates is:

$$
\rho\colon \mathbb{R}_{\geq 0} \rightarrow \mathbb{R}_{>0}, \quad r \mapsto \rho(r)
= \frac{3Ma^2}{4\pi (a^2 + r^2)^\frac{5}{2}}
$$

where $a \in \mathbb{R}_{>0}$ is the core radius (not the entire cluster) and $M \in \mathbb{R}_{>0}$ is the total mass of the cluster.

To sample a position for body $i$, we use this density as a probability distribution and apply inverse transform sampling. First, we normalize the density by dividing by $M$ and calculating the cumulative distribution function (CDF):

$$
\mathrm{CDF}(r)
= \int_0^r \mathrm{d} r' \, r'^2 \int_0^{\pi} \mathrm{d}\vartheta \, \sin(\vartheta) \int_0^{2\pi} \mathrm{d}\varphi \, \frac{\rho(r')}{M}
= \frac{4\pi}{M} \int_0^r \mathrm{d}r' \, r'^2 \rho(r') = \frac{r^3}{(a^2 + r^2)^{\frac{3}{2}}}
$$

Setting $\mathrm{CDF}(r) = u$, where $u \sim \mathrm{Unif}([0, 1])$, and solving for $r$ gives:

$$
r = \frac{a}{\left( u^{-\frac{2}{3}} - 1 \right)^{\frac{1}{2}}}
$$

We now have a uniformly sampled radius in spherical coordinates. The angles $\vartheta$ and $\varphi$ are sampled uniformly using $\vartheta = \cos^{-1}(2w_1 - 1)$ and $\varphi = 2\pi w_2$, where $w_i \sim \mathrm{Unif}([0, 1])$ for $i \in \{1, 2\}$ (see \[4\]). These spherical coordinates are then transformed into Cartesian coordinates:

$$
\mathbf{x}_i(0) =
\begin{pmatrix}
  r \sin(\vartheta) \cos(\varphi) \\
  r \sin(\vartheta) \sin(\varphi) \\
  r \cos(\vartheta)
\end{pmatrix}
$$

To sample the velocity, we use the energy distribution function of the Plummer model:

$$
f\colon \mathbb{R} \rightarrow \mathbb{R}, \quad E \mapsto f(E) =
\begin{cases}
  \frac{24\sqrt{2}}{7\pi^3} \frac{a^2}{G^5 M^4}(-E^\frac{7}{2}) & \text{for } E<0 \\
  0 & \text{for } E \geq 0
\end{cases}
$$

The specific energy for body $i$ is $E_i = \frac{1}{2} \Vert \mathbf{v}_i(0) \Vert^2 + \Phi(r)$, where $\Phi(r) = -\frac{GM}{\sqrt{a^2 + r^2}}$ is the potential. According to the energy distribution function the maximum velocity a body can attain is at $E_i = 0$, the probability of finding a body $i$ with a higher velocity is zero:

$$
\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert = \sqrt{-2\Phi(r)} = \sqrt{\frac{2GM}{\sqrt{a^2 + r^2}}}
$$

The maximum energy is then:

$$
E_{i,\mathrm{max}}
= \frac{1}{2} \Vert \mathbf{v}_i(0) \Vert^2 + \Phi(r)
= \frac{1}{2} \Vert \mathbf{v}_i(0) \Vert^2 + \frac{1}{2} \Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2
= -\frac{1}{2} \Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2 \left( 1 - \frac{\Vert \mathbf{v}_i(0) \Vert^2}{\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2} \right)
$$

Using the energy distribution as a probability density and accounting for the velocity volume element $4\pi v^2\, \mathrm{d}v$, the probability of finding a body $i$ with a specific energy up to $E_{i,\mathrm{max}}$ is proportional to:

$$f(E_{i,\mathrm{max}}) \propto (-E_{i,\mathrm{max}})^{\frac{7}{2}} \propto \left( 1 - \frac{\Vert \mathbf{v}_i(0) \Vert^2}{\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2} \right)^{\frac{7}{2}}$$

We use this relationship to sample the velocity via rejection sampling:

1. Calculate $\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert$ for body $i$ based on its sampled radius $r$.
2. Sample a candidate velocity magnitude uniformly, i.e. $\Vert \mathbf{v}_i(0) \Vert \sim \mathrm{Unif}([0, \Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert])$.
3. Sample a random variable $\xi \sim \mathrm{Unif}([0, 1])$.
  * If $\xi \leq \left( 1 - \frac{\Vert \mathbf{v}_i(0) \Vert^2}{\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2} \right)^{\frac{7}{2}}$, the velocity magnitude is accepted. The velocity vector $\mathbf{v}_i(0)$ is then constructed by sampling directions $\vartheta$ and $\varphi$ uniformly, as was done for the position vector:
  $$
  \mathbf{v}_i(0) =
  \begin{pmatrix}
    \Vert \mathbf{v}_i(0) \Vert \sin(\vartheta) \cos(\varphi) \\
    \Vert \mathbf{v}_i(0) \Vert \sin(\vartheta) \sin(\varphi) \\
    \Vert \mathbf{v}_i(0) \Vert \cos(\vartheta)
  \end{pmatrix}
  $$
  * If $\xi > \left( 1 - \frac{\Vert \mathbf{v}_i(0) \Vert^2}{\Vert \mathbf{v}_{i, \mathrm{max}}(0) \Vert^2} \right)^{\frac{7}{2}}$, the candidate is rejected. Start again from step 2.

Once positions and velocities are sampled for all bodies, the center of mass and net system velocity are set to zero to avoid drifting, as described in a previous section.

For more information about the Plummer model, see \[5\].

## Implementation

### Technology Stack

The primary programming language is C++23. While the core requirements of an n-body simulation could be met with older standards, C++23 was selected to experiment with contemporary language features, such as ranges and concepts, even in instances where their application may seem 'too much'. The project is managed via CMake for build orchestration and vcpkg for dependency management.

For visualization and GPU computation, OpenGL 4.6 is employed. This version was chosen to utilize modern features such as Direct State Access (DSA) and persistent mapping, which reduce driver overhead and simplify buffer management. While the use of compute shaders only requires OpenGL 4.3, the higher version allows for a more streamlined implementation of the graphics pipeline.

To keep the project lightweight and maintainable, third-party dependencies are kept to a minimum and standard library features are prioritized. However, third-party dependencies are used where implementing equivalent functionality would be counterproductive to the project's primary goals:

* **GLM**: Used for linear algebra and vector mathematics, providing an API closely aligned with GLSL.
* **GLFW**: Used for window creation and the handling of keyboard and mouse input.
* **glad**: Used as an OpenGL loader to access modern function pointers.

### Architecture

#### Overview

The architecture is designed to support two distinct simulation backends: one targeting the CPU and one targeting the GPU. The CPU implementation leverages multi-threading and the Barnes-Hut algorithm to optimize force calculations, whereas the GPU implementation utilizes compute shaders to execute a naive all-pairs approach in parallel. Regardless of the chosen backend, the resulting state of the system is rendered in real-time using OpenGL.

#### Design Goals

The architecture was guided by two primary objectives:

1. **Unification of Simulation Backends**: It was important to avoid creating two entirely independent implementations for the CPU and GPU. While the underlying algorithms (Barnes-Hut vs. all-pairs) differ significantly, they both solve the same physical problem and should therefore share a common interface. This approach ensures synergy between the components and allows the rendering logic to remain agnostic of the simulation source.
2. **Decoupling of Simulation and Rendering**: The process of calculating the system's state (simulation) is logically distinct from how that state is visualized (rendering). By decoupling these concerns, the architecture remains extensible. For instance, the simulation output could be redirected to a file for post-processing without requiring modifications to the core simulation logic.

#### Technical Realization

The design goals were achieved by centering the architecture around a Shader Storage Buffer Object (SSBO) residing on the GPU. This buffer serves as the 'single source of truth' for the current state of the system. The link between the simulation and rendering logic is a handle to this SSBO, which allows the renderer to access data without needing knowledge of the producer's identity or internal logic.

The implementation is organized into the following key components:

* `Renderer`: This class is responsible for the visual representation of the bodies. It operates exclusively on the SSBO handle and knows only the memory layout of the data required for rendering. It does not manage the buffer's lifecycle nor does it possess knowledge of how the data was generated. It simply reads the current state to draw circles on the screen.
* `Simulatable`: This is a C++ concept that defines the interface for any simulation backend. Both `SimulationCPU` and `SimulationGPU` implement this concept, ensuring they provide the necessary methods for updating the system state and managing the SSBO buffer.

The distinction in data ownership between the two backends is important:

* `SimulationCPU`: The CPU maintains primary ownership of the simulation data in host memory. After each integration step, the updated data is copied from host memory to the SSBO on device memory. In this mode, the GPU holds a read-only mirror of the state for rendering purposes.
* `SimulationGPU`: The GPU owns the data directly within the SSBO on device memory. The compute shaders update the buffer in-place. To maximize performance and avoid memory transfer bottlenecks, the data is never copied back to host memory. The CPU merely holds a handle to the buffer to pass it to the renderer.

#### GPU Implementation

The implementation for the simulation on the GPU is based on \[6\]. Although this article provides an implementation for an all-pairs simulation in CUDA and ours uses compute shaders in OpenGL, the general structure and optimizations such as tiled computations, coalesced memory accesses and using shared memory are based on this article.

The GPU simulation is based on the tiled approach described in \[6\]. Although the original reference implements the all-pairs algorithm using CUDA, for this project the logic was translated into OpenGL compute shaders. As in the original reference, several optimizations were employed:

* **Tiled Computations**: The workload is divided into tiles to maximize data reuse. A work group processes multiple tiles.
* **Shared Memory**: By loading a tile of body data into the GPU's fast local shared memory before processing, we significantly reduce the number of expensive global memory accesses per invocation.
* **Coalesced Memory Access**: Data structures are aligned to ensure that memory reads are coalesced, minimizing the number of memory transactions required to fetch data.

## References

1. [https://en.wikipedia.org/wiki/N-body\_problem](https://en.wikipedia.org/wiki/N-body_problem)
2. [https://en.wikipedia.org/wiki/Verlet\_integration](https://en.wikipedia.org/wiki/Verlet_integration)
3. [https://beltoforion.de/en/barnes-hut-galaxy-simulator/](https://beltoforion.de/en/barnes-hut-galaxy-simulator/)
4. [https://mathworld.wolfram.com/SpherePointPicking.html](https://mathworld.wolfram.com/SpherePointPicking.html)
5. [https://en.wikipedia.org/wiki/Plummer_model](https://en.wikipedia.org/wiki/Plummer_model)
6. [https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda)
