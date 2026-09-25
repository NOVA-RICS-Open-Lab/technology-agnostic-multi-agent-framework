import pandas as pd
import matplotlib.pyplot as plt
import os

def main():
    results_path = os.path.join(os.path.dirname(__file__), 'comparison_results.txt')
    
    df_mk = pd.read_csv(results_path, sep='\t')
    
    tamaf_python = df_mk[df_mk['Framework'].str.upper() == 'TAMAF_PYTHON']
    tamaf_java = df_mk[df_mk['Framework'].str.upper() == 'TAMAF_JAVA']
    jade = df_mk[df_mk['Framework'].str.upper() == 'JADE']

    output_dir = os.path.dirname(__file__)
    
    # 1. Plot Registration Time vs Scale
    plt.figure(figsize=(8, 5))
    plt.plot(tamaf_python['Scale'], tamaf_python['Avg_Registration_Time_s'], marker='o', label='TAMAF (Python)', linestyle='-', color='#1f77b4')
    plt.plot(tamaf_java['Scale'], tamaf_java['Avg_Registration_Time_s'], marker='^', label='TAMAF (Java)', linestyle='-.', color='#2ca02c')
    plt.plot(jade['Scale'], jade['Avg_Registration_Time_s'], marker='s', label='JADE', linestyle='--', color='#ff7f0e')
    
    plt.title('Registration Time vs Number of Agents')
    plt.xlabel('Number of Sender Agents')
    plt.ylabel('Registration Time (seconds)')
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'registration_time.pdf'))
    plt.close()
    
    # 2. Plot Average RTT vs Scale
    plt.figure(figsize=(8, 5))
    plt.plot(tamaf_python['Scale'], tamaf_python['Grand_Avg_RTT_s'], marker='o', label='TAMAF (Python)', linestyle='-', color='#1f77b4')
    plt.plot(tamaf_java['Scale'], tamaf_java['Grand_Avg_RTT_s'], marker='^', label='TAMAF (Java)', linestyle='-.', color='#2ca02c')
    plt.plot(jade['Scale'], jade['Grand_Avg_RTT_s'], marker='s', label='JADE', linestyle='--', color='#ff7f0e')
    
    plt.title('Average Round Trip Time vs Number of Agents')
    plt.xlabel('Number of Sender Agents')
    plt.ylabel('Average RTT (seconds)')
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'avg_rtt.pdf'))
    plt.close()

    print(f"Plots saved to {output_dir}")

if __name__ == "__main__":
    main()                                                                                                                       