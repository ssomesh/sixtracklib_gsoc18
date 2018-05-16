# Hamiltonian


$$
H =- (1-hx) \sqrt{(E^2 -\Phi^2) - (P_x-A_x)^2 - (P_y-A_y)^2-m^2c^4}+A_z
$$


$$
p=\partial_q F_2(q,P,t) \qquad
Q=\partial_P F_2(q,P,t) \qquad
K=H+\partial_t F_2(q,P,t)
$$
$$
F_2= (s - t/(c\beta_0) ) \sqrt{P^2c^2 +m^2 c^4}
$$
$$
z=(s/\beta_0 - ct) \frac{P}{\sqrt{P^2c^2 +m^2 c^4}}= \beta (s/\beta_0 -ct  )  \qquad
E=\sqrt{P^2c^2 +m^2 c^4}
$$

$$
F_2= (s/\beta_0 - ct)  \sqrt{(\Delta P-P_0)^2 c^2+m^2 c^4}
$$

$$
z=(s/\beta_0 - ct) \frac{P}{\sqrt{(\Delta P-P_0)^2c^2 +m^2 c^4}}= \beta (s/\beta_0 -ct  )  \qquad
E=\sqrt{(\Delta P-P_0)^2c^2 +m^2 c^4}
$$

$$
H =\frac{\beta}{\beta_0}\delta - (1-hx) \sqrt{((1+\delta)^2 - (p_x-a_x)^2 - (p_y-a_y)^2}+ a_z
$$

Constants: 
$$
q, m, \beta_0, P_0, E_0
$$

Variables:
$$
\begin{aligned}
x&  & p_x& =P_x/P_0 \\ 
y&  & p_y&=P_y/P_0  \\
z& =\beta ( s/\beta_0 -  c t)  & \delta &=\frac{P-P_0}{P_0}
 \end{aligned}
$$

Extra variables:
$$
\begin{aligned}
\delta&=P/P_0 -1 & r_p&= P_0/P\qquad r_v= \beta/\beta_0 
\end{aligned}
$$

# Tracking equations



RF Cavity:
$$
\begin{aligned}
t_0& =s/(\beta_0 c) &
\Delta E &= V \sin (2 \pi (f+\delta_f) (t_0 - t) + \varphi)  = V \sin (2 \pi f z/ (\beta c) + \varphi)
\\
 P &= \sqrt{E^2 - m^2} &
 \beta &= E/ P \\
 r_p &= P_0/P  &
 r_v &= \beta_0/\beta= \beta_0 E/P\  &
 \delta & = 1/r_p-1
 \end{aligned}
$$

Drift:
$$
\begin{aligned}
(x_p,y_p)&=(p_x, p_y) r_p \\
\Delta (x,y) &= L \left(p_x, p_y\right) r_p &
   \Delta z &=
        L \left( r_v- 
          \left( 1 +\frac{x_p^2 + y_p^2}{ 2}\right) \right) 
\end{aligned}
$$

Drift exact:
$$
\begin{aligned}
p_z&=\sqrt{(1+\delta)^2-p_x^2-p_y^2} &
\beta_z&=\frac{P_0 p_z}{E} \\
 \Delta (x,y) &= L \frac{\left(p_x,p_y\right)}{p_z} &
   \Delta z &=
        L r_v - \frac{L}{\beta_z}=L r_v - L\frac{p_z}{E}
\end{aligned}
$$


Multipole:
$$
\Delta (p_x + i p_y) = q \sum_n (k_n + i \bar k_n) (x + i y)^n
$$

Thin dipole full: 
$$
\Delta p_x =  L \left(h/r_p  - q k_1 (1+  h x) \right)\qquad
   \Delta \sigma = - L h r_v
$$

Thin dipole : 
$$
\Delta p_x =  L \left(h \delta - h^2 x \right)\qquad
   \Delta \sigma = - L h r_v
$$






# Useful formulas

$$
\begin{aligned}
\delta&=\frac{P-P_0}{P_0} &
p_t&=\frac{E-E_0}{P_0c} &
p_\sigma&=\frac{E-E_0}{\beta_0 P_0c}
\end{aligned}
$$

$$
\begin{aligned}
\delta&=\sqrt{p_t^2 + 2 p_t/\beta_0 +1} -1 &
\frac{d \delta}{d p_t}= \frac{p_t+1/\beta_0}{1+\delta} = \frac{1}{\beta} \\
\delta&=\sqrt{\beta_0^2 p_\sigma^2 + 2 p_\sigma +1} -1 &
\frac{d \delta}{d p_\sigma}= \frac{p_\sigma\beta_0^2+1}{1+\delta} = \frac{\beta_0}{\beta} 
\end{aligned}
$$


$$
\begin{aligned}
  p_z&=\sqrt{(1+\delta)^2 - p_x^2 - p_y^2} \\
  \frac{d p_z}{d p_t}&= \frac{p_t+1/\beta_0}{p_z} = \frac{1}{\beta_z}  \\
  \frac{d p_z}{d p_\sigma}&= \frac{\beta_0^2 p_\sigma+1}{p_z} = \frac{\beta_0}{\beta_z}  
\end{aligned}
$$


