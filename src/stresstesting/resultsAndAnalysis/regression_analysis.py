import os
import numpy as np
import pandas as pd
from scipy.optimize import curve_fit

current_dir = os.path.dirname(os.path.abspath(__file__))
data_path = os.path.join(current_dir, "comparison_results.txt")
if not os.path.exists(data_path):
    raise FileNotFoundError(f"Could not find dataset at: {data_path}")
df = pd.read_csv(data_path, sep="\t")

jade_df = df[df["Framework"].str.upper() == "JADE"].reset_index(drop=True)
py_df = df[df["Framework"].str.upper() == "TAMAF_PYTHON"].reset_index(drop=True)
java_df = df[df["Framework"].str.upper() == "TAMAF_JAVA"].reset_index(drop=True)
N = py_df["Scale"].values

def load_data(registration_rtt: bool):
    specificdatatoget = 'Grand_Avg_RTT_s'
    if registration_rtt:
        specificdatatoget = 'Avg_Registration_Time_s'
    return py_df['Scale'].values, py_df[specificdatatoget].values, java_df['Scale'].values, java_df[specificdatatoget].values, jade_df['Scale'].values, jade_df[specificdatatoget].values

def get_r2(y_true, y_pred):
    ss_res = np.sum((y_true - y_pred)**2)
    ss_tot = np.sum((y_true - np.mean(y_true))**2)
    return 1 - (ss_res / ss_tot)

def get_rmse(y_true, y_pred):
    return np.sqrt(np.mean((y_true - y_pred)**2))

def power_law(x, a, b):
    return a * (x**b)

def get_equations(x, y):
    # 2. Linear Model (y = ax + b)
    lin_a, lin_b = np.polyfit(x, y, 1)
    y_pred_lin = (lin_a * x) + lin_b
    print(f"Linear:    y = {lin_a:.4f}x + {lin_b:.4f}  (R² = {get_r2(y, y_pred_lin):.4f}, RMSE = {get_rmse(y, y_pred_lin):.4f})")

    # 3. Quadratic Model (y = ax^2 + bx + c)
    quad_a, quad_b, quad_c = np.polyfit(x, y, 2)
    y_pred_quad = (quad_a * x**2) + (quad_b * x) + quad_c
    print(f"Quadratic: y = {quad_a:.4f}x² + {quad_b:.4f}x + {quad_c:.4f}  (R² = {get_r2(y, y_pred_quad):.4f}, RMSE = {get_rmse(y, y_pred_quad):.4f})")

    # 4. Power Law Model (y = a * x^b)
    try:
        popt, _ = curve_fit(power_law, x, y)
        y_pred_power = power_law(x, popt[0], popt[1])
        print(f"Power Law: y = {popt[0]:.4f} * x^{popt[1]:.4f}  (R² = {get_r2(y, y_pred_power):.4f}, RMSE = {get_rmse(y, y_pred_power):.4f})")
    except Exception as e:
        print(f"Power Law: Failed to fit. Ensure 'Scale' contains no 0 or negative values. Error: {e}")


print("--- Registration Time ---")
x_py, y_py, x_java, y_java, x_jade, y_jade = load_data(True)
print("--- Mathematical Equations for Jade ---")
get_equations(x_jade, y_jade)
print("--- Mathematical Equations for Python ---")
get_equations(x_py, y_py)
print("--- Mathematical Equations for Java ---")
get_equations(x_java, y_java)
print("--- RTT ---")
x_py, y_py, x_java, y_java, x_jade, y_jade = load_data(False)
print("--- Mathematical Equations for Jade ---")
get_equations(x_jade, y_jade)
print("--- Mathematical Equations for Python ---")
get_equations(x_py, y_py)
print("--- Mathematical Equations for Java ---")
get_equations(x_java, y_java)
