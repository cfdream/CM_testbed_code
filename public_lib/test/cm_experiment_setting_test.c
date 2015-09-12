#include <stdio.h>
#include "../cm_experiment_setting.h"

extern cm_experiment_setting_t cm_experiment_setting;
int main() {
    init_cm_experiment_setting();
    int max_switch_map_size = (int)(
                                cm_experiment_setting.switch_memory_times
                                * cm_experiment_setting.sample_hold_setting.default_byte_sampling_rate
                                * cm_experiment_setting.sample_hold_setting.max_switch_interval_volume
                                * cm_experiment_setting.sample_hold_setting.uniform_mem_ratio_to_diverse_mem
                                ) ;
    printf("interval_msec_len:%d \n \
           condition_msec_freq:%d \n \
           replacement:%d \n \
           switch_mem_type:%d \n \
           switch_drop_rate:%f \n \
           uniform_mem_ratio_to_diverse_mem:%f \n \
           max_switch_map_size:%d \n \
           ",
           cm_experiment_setting.interval_msec_len,
           cm_experiment_setting.condition_msec_freq,
           cm_experiment_setting.replacement,
           cm_experiment_setting.switch_mem_type,
           cm_experiment_setting.switch_drop_rate,
           cm_experiment_setting.sample_hold_setting.uniform_mem_ratio_to_diverse_mem,
           max_switch_map_size);
    int i = 0;
    for (i=0; i < NUM_SWITCHES; i++) {
        printf("switch%d_interval_volume:%lu\n", i+1, cm_experiment_setting.sample_hold_setting.switches_interval_volume[i]);
    }
}
