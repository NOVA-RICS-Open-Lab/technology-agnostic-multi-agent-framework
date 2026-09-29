import glob

scales = [1, 5, 10, 15, 20, 30, 40, 50, 75, 100, 200]
# frameworks = ['jade']
# frameworks = ['tamaf_python', 'tamaf_java']
frameworks = ['jade', 'tamaf_python', 'tamaf_java']

def parse_single_file(filepath):
    with open(filepath, 'r') as f:
        lines = f.readlines()
        
    reg_time = None
    rtts = []
    timeouts = 0
    
    for line in lines:
        if line.startswith("Registration Time:"):
            reg_time = float(line.split(":")[1].strip())
        elif ":" in line and not line.startswith("Round Trip Times"):
            val = line.rsplit(":", 1)[1].strip()
            if val == "TIMEOUT":
                timeouts += 1
            else:
                try:
                    rtts.append(float(val))
                except ValueError:
                    pass
                    
    avg_rtt = sum(rtts) / len(rtts) if len(rtts) > 0 else 0
    total = len(rtts) + timeouts
    return reg_time, avg_rtt, timeouts, total

with open("comparison_results.txt", "w") as out:
    out.write("Scale\tFramework\tRuns\tAvg_Registration_Time_s\tGrand_Avg_RTT_s\tAvg_Timeouts\n")
    
    for scale in scales:
        for fw in frameworks:
            file_pattern = f"../{fw}/results/*_test_{scale}_agent_testresults.txt"
            filepaths = glob.glob(file_pattern)
            
            if not filepaths:
                continue 
            
            all_reg_times = []
            all_avg_rtts = []
            all_timeouts = []
            all_totals = []
            
            for filepath in filepaths:
                res = parse_single_file(filepath)
                if res and res[0] is not None:
                    reg_time, file_avg_rtt, timeouts, total = res
                    all_reg_times.append(reg_time)
                    all_avg_rtts.append(file_avg_rtt)
                    all_timeouts.append(timeouts)
                    all_totals.append(total)
            
            num_runs = len(all_reg_times)
            if num_runs > 0:
                grand_avg_reg_time = sum(all_reg_times) / num_runs
                grand_avg_rtt = sum(all_avg_rtts) / num_runs
                grand_avg_timeouts = sum(all_timeouts) / num_runs
                grand_avg_total = sum(all_totals) / num_runs
                
                out.write(f"{scale}\t{fw.upper()}\t{num_runs}\t{grand_avg_reg_time:.4f}\t{grand_avg_rtt:.4f}\t{grand_avg_timeouts:.1f}/{grand_avg_total:.1f}\n")

print("Consolidation complete.")