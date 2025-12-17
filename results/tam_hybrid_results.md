| timestamp | machine_id | algo | approach | n | nprocs | nthreads | repetitions | time_sec | time_min | time_max | time_mean | gflops_gemm_eq | passed | speedup_vs_naive | note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2025-12-12T13:52:58Z | kiet | naive | hybrid | 128 | 2 | 4 | 1 | 0.000580 | 0.000580 | 0.000580 | 0.000580 | 7.2279 | true | 0.0000 | report sweep |
| 2025-12-12T13:52:58Z | kiet | naive | hybrid | 256 | 2 | 4 | 1 | 0.005320 | 0.005320 | 0.005320 | 0.005320 | 6.3076 | true | 0.0000 | report sweep |
| 2025-12-12T13:52:59Z | kiet | naive | hybrid | 512 | 2 | 4 | 1 | 0.060519 | 0.060519 | 0.060519 | 0.060519 | 4.4355 | true | 0.0000 | report sweep |
| 2025-12-12T13:53:01Z | kiet | naive | hybrid | 1024 | 2 | 4 | 1 | 0.519495 | 0.519495 | 0.519495 | 0.519495 | 4.1338 | true | 0.0000 | report sweep |
| 2025-12-12T13:54:28Z | kiet | naive | hybrid | 2048 | 2 | 4 | 1 | 17.648955 | 17.648955 | 17.648955 | 17.648955 | 0.9734 | true | 0.0000 | report sweep |
| 2025-12-12T13:54:28Z | kiet | strassen | hybrid | 128 | 2 | 4 | 1 | 0.000919 | 0.000919 | 0.000919 | 0.000919 | 4.5642 | true | 0.0000 | report sweep |
| 2025-12-12T13:54:28Z | kiet | strassen | hybrid | 256 | 2 | 4 | 1 | 0.005775 | 0.005775 | 0.005775 | 0.005775 | 5.8106 | true | 0.0000 | report sweep |
| 2025-12-12T13:54:28Z | kiet | strassen | hybrid | 512 | 2 | 4 | 1 | 0.041357 | 0.041357 | 0.041357 | 0.041357 | 6.4908 | true | 0.0000 | report sweep |
| 2025-12-12T13:54:30Z | kiet | strassen | hybrid | 1024 | 2 | 4 | 1 | 0.277244 | 0.277244 | 0.277244 | 0.277244 | 7.7458 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:09Z | kiet | strassen | hybrid | 2048 | 2 | 4 | 1 | 1.939355 | 1.939355 | 1.939355 | 1.939355 | 8.8585 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:09Z | kiet | proposed | hybrid | 128 | 2 | 4 | 1 | 0.000456 | 0.000456 | 0.000456 | 0.000456 | 9.2022 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:09Z | kiet | proposed | hybrid | 256 | 2 | 4 | 1 | 0.004000 | 0.004000 | 0.004000 | 0.004000 | 8.3893 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:09Z | kiet | proposed | hybrid | 512 | 2 | 4 | 1 | 0.030107 | 0.030107 | 0.030107 | 0.030107 | 8.9161 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:11Z | kiet | proposed | hybrid | 1024 | 2 | 4 | 1 | 0.231568 | 0.231568 | 0.231568 | 0.231568 | 9.2737 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:54Z | kiet | proposed | hybrid | 2048 | 2 | 4 | 1 | 1.796997 | 1.796997 | 1.796997 | 1.796997 | 9.5603 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:54Z | kiet | naive | hybrid | 128 | 4 | 2 | 1 | 0.000474 | 0.000474 | 0.000474 | 0.000474 | 8.8541 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:54Z | kiet | naive | hybrid | 256 | 4 | 2 | 1 | 0.001979 | 0.001979 | 0.001979 | 0.001979 | 16.9562 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:54Z | kiet | naive | hybrid | 512 | 4 | 2 | 1 | 0.020916 | 0.020916 | 0.020916 | 0.020916 | 12.8341 | true | 0.0000 | report sweep |
| 2025-12-12T13:55:56Z | kiet | naive | hybrid | 1024 | 4 | 2 | 1 | 0.209446 | 0.209446 | 0.209446 | 0.209446 | 10.2532 | true | 0.0000 | report sweep |
| 2025-12-12T13:56:51Z | kiet | naive | hybrid | 2048 | 4 | 2 | 1 | 6.343147 | 6.343147 | 6.343147 | 6.343147 | 2.7084 | true | 0.0000 | report sweep |
| 2025-12-12T13:56:51Z | kiet | strassen | hybrid | 128 | 4 | 2 | 1 | 0.001007 | 0.001007 | 0.001007 | 0.001007 | 4.1651 | true | 0.0000 | report sweep |
| 2025-12-12T13:56:51Z | kiet | strassen | hybrid | 256 | 4 | 2 | 1 | 0.004350 | 0.004350 | 0.004350 | 0.004350 | 7.7134 | true | 0.0000 | report sweep |
| 2025-12-12T13:56:52Z | kiet | strassen | hybrid | 512 | 4 | 2 | 1 | 0.027369 | 0.027369 | 0.027369 | 0.027369 | 9.8080 | true | 0.0000 | report sweep |
| 2025-12-12T13:56:53Z | kiet | strassen | hybrid | 1024 | 4 | 2 | 1 | 0.188611 | 0.188611 | 0.188611 | 0.188611 | 11.3858 | true | 0.0000 | report sweep |
| 2025-12-12T13:57:33Z | kiet | strassen | hybrid | 2048 | 4 | 2 | 1 | 1.253451 | 1.253451 | 1.253451 | 1.253451 | 13.7061 | true | 0.0000 | report sweep |
| 2025-12-12T13:57:33Z | kiet | proposed | hybrid | 128 | 4 | 2 | 1 | 0.000303 | 0.000303 | 0.000303 | 0.000303 | 13.8616 | true | 0.0000 | report sweep |
| 2025-12-12T13:57:33Z | kiet | proposed | hybrid | 256 | 4 | 2 | 1 | 0.001815 | 0.001815 | 0.001815 | 0.001815 | 18.4868 | true | 0.0000 | report sweep |
| 2025-12-12T13:57:33Z | kiet | proposed | hybrid | 512 | 4 | 2 | 1 | 0.011013 | 0.011013 | 0.011013 | 0.011013 | 24.3744 | true | 0.0000 | report sweep |
| 2025-12-12T13:57:35Z | kiet | proposed | hybrid | 1024 | 4 | 2 | 1 | 0.077454 | 0.077454 | 0.077454 | 0.077454 | 27.7261 | true | 0.0000 | report sweep |
| 2025-12-12T13:58:14Z | kiet | proposed | hybrid | 2048 | 4 | 2 | 1 | 0.550266 | 0.550266 | 0.550266 | 0.550266 | 31.2210 | true | 0.0000 | report sweep |
| 2025-12-12T13:58:14Z | kiet | naive | hybrid | 128 | 8 | 1 | 1 | 0.000238 | 0.000238 | 0.000238 | 0.000238 | 17.6540 | true | 0.0000 | report sweep |
| 2025-12-12T13:58:14Z | kiet | naive | hybrid | 256 | 8 | 1 | 1 | 0.001852 | 0.001852 | 0.001852 | 0.001852 | 18.1211 | true | 0.0000 | report sweep |
| 2025-12-12T13:58:14Z | kiet | naive | hybrid | 512 | 8 | 1 | 1 | 0.023109 | 0.023109 | 0.023109 | 0.023109 | 11.6159 | true | 0.0000 | report sweep |
| 2025-12-12T13:58:16Z | kiet | naive | hybrid | 1024 | 8 | 1 | 1 | 0.259519 | 0.259519 | 0.259519 | 0.259519 | 8.2749 | true | 0.0000 | report sweep |
| 2025-12-12T13:59:16Z | kiet | naive | hybrid | 2048 | 8 | 1 | 1 | 6.275499 | 6.275499 | 6.275499 | 6.275499 | 2.7376 | true | 0.0000 | report sweep |
| 2025-12-12T13:59:16Z | kiet | strassen | hybrid | 128 | 8 | 1 | 1 | 0.001045 | 0.001045 | 0.001045 | 0.001045 | 4.0146 | true | 0.0000 | report sweep |
| 2025-12-12T13:59:16Z | kiet | strassen | hybrid | 256 | 8 | 1 | 1 | 0.006904 | 0.006904 | 0.006904 | 0.006904 | 4.8602 | true | 0.0000 | report sweep |
| 2025-12-12T13:59:16Z | kiet | strassen | hybrid | 512 | 8 | 1 | 1 | 0.046582 | 0.046582 | 0.046582 | 0.046582 | 5.7626 | true | 0.0000 | report sweep |
| 2025-12-12T13:59:19Z | kiet | strassen | hybrid | 1024 | 8 | 1 | 1 | 0.310910 | 0.310910 | 0.310910 | 0.310910 | 6.9071 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:04Z | kiet | strassen | hybrid | 2048 | 8 | 1 | 1 | 2.318385 | 2.318385 | 2.318385 | 2.318385 | 7.4103 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:04Z | kiet | proposed | hybrid | 128 | 8 | 1 | 1 | 0.000493 | 0.000493 | 0.000493 | 0.000493 | 8.5048 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:04Z | kiet | proposed | hybrid | 256 | 8 | 1 | 1 | 0.001908 | 0.001908 | 0.001908 | 0.001908 | 17.5895 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:04Z | kiet | proposed | hybrid | 512 | 8 | 1 | 1 | 0.012447 | 0.012447 | 0.012447 | 0.012447 | 21.5670 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:06Z | kiet | proposed | hybrid | 1024 | 8 | 1 | 1 | 0.078943 | 0.078943 | 0.078943 | 0.078943 | 27.2030 | true | 0.0000 | report sweep |
| 2025-12-12T14:00:47Z | kiet | proposed | hybrid | 2048 | 8 | 1 | 1 | 0.583418 | 0.583418 | 0.583418 | 0.583418 | 29.4469 | true | 0.0000 | report sweep |
| 2025-12-16T14:02:55Z | kiet | naive | hybrid | 128 | 2 | 4 | 5 | 0.000596 | 0.000584 | 0.000687 | 0.000629 | 7.0414 | true | 1.8578 | rerun pinned |
| 2025-12-16T14:02:55Z | kiet | naive | hybrid | 256 | 2 | 4 | 5 | 0.005766 | 0.005627 | 0.006014 | 0.005794 | 5.8193 | true | 1.8840 | rerun pinned |
| 2025-12-16T14:02:56Z | kiet | naive | hybrid | 512 | 2 | 4 | 5 | 0.061008 | 0.060592 | 0.061844 | 0.061114 | 4.4000 | true | 1.9414 | rerun pinned |
| 2025-12-16T14:03:00Z | kiet | naive | hybrid | 1024 | 2 | 4 | 5 | 0.533430 | 0.531466 | 0.537436 | 0.534394 | 4.0258 | true | 1.8294 | rerun pinned |
| 2025-12-16T14:34:28Z | kiet | naive | hybrid | 2048 | 4 | 2 | 5 | 10.453979 | 10.434314 | 10.482155 | 10.456534 | 1.6434 | true | 3.4511 | rerun pinned |
| 2025-12-16T14:35:17Z | kiet | strassen | hybrid | 2048 | 4 | 2 | 5 | 2.020878 | 2.014514 | 2.031994 | 2.023230 | 8.5012 | true | 18.3069 | rerun pinned |
| 2025-12-16T14:35:58Z | kiet | proposed | hybrid | 2048 | 4 | 2 | 5 | 0.971327 | 0.969019 | 0.973078 | 0.971239 | 17.6870 | true | 36.0358 | rerun pinned |
| 2025-12-17T08:30:38Z | unknown | naive | hybrid | 128 | 2 | 4 | 5 | 0.004155 | 0.004048 | 0.004328 | 0.004172 | 1.0093 | true | 1.8144 | final_report_sweep |
| 2025-12-17T08:30:39Z | unknown | naive | hybrid | 256 | 2 | 4 | 5 | 0.041436 | 0.040120 | 0.043778 | 0.041613 | 0.8098 | true | 1.7908 | final_report_sweep |
| 2025-12-17T08:30:44Z | unknown | naive | hybrid | 512 | 2 | 4 | 5 | 0.531974 | 0.529417 | 0.550560 | 0.538187 | 0.5046 | true | 1.6751 | final_report_sweep |
| 2025-12-17T08:31:51Z | unknown | naive | hybrid | 1024 | 2 | 4 | 5 | 8.139842 | 7.532067 | 8.350767 | 8.015451 | 0.2638 | true | 1.4494 | final_report_sweep |
| 2025-12-17T08:43:50Z | unknown | naive | hybrid | 2048 | 2 | 4 | 5 | 84.273781 | 83.141847 | 84.663786 | 84.184061 | 0.2039 | true | 1.5246 | final_report_sweep |
| 2025-12-17T08:43:50Z | unknown | strassen | hybrid | 128 | 2 | 4 | 5 | 0.005668 | 0.005328 | 0.006657 | 0.005808 | 0.7400 | true | 1.2802 | final_report_sweep |
| 2025-12-17T08:43:51Z | unknown | strassen | hybrid | 256 | 2 | 4 | 5 | 0.028531 | 0.025852 | 0.037263 | 0.029705 | 1.1761 | true | 2.5161 | final_report_sweep |
| 2025-12-17T08:43:53Z | unknown | strassen | hybrid | 512 | 2 | 4 | 5 | 0.223723 | 0.206295 | 0.232737 | 0.219520 | 1.1999 | true | 3.8461 | final_report_sweep |
| 2025-12-17T08:44:14Z | unknown | strassen | hybrid | 1024 | 2 | 4 | 5 | 1.441641 | 1.414723 | 1.470071 | 1.439591 | 1.4896 | true | 7.3309 | final_report_sweep |
| 2025-12-17T08:47:32Z | unknown | strassen | hybrid | 2048 | 2 | 4 | 5 | 10.347628 | 9.634967 | 10.563150 | 10.171113 | 1.6603 | true | 12.1253 | final_report_sweep |
| 2025-12-17T08:47:33Z | unknown | proposed | hybrid | 128 | 2 | 4 | 5 | 0.003077 | 0.001819 | 0.003762 | 0.002882 | 1.3632 | true | 2.6029 | final_report_sweep |
| 2025-12-17T08:47:33Z | unknown | proposed | hybrid | 256 | 2 | 4 | 5 | 0.024702 | 0.022112 | 0.025591 | 0.024289 | 1.3584 | true | 3.0355 | final_report_sweep |
| 2025-12-17T08:47:35Z | unknown | proposed | hybrid | 512 | 2 | 4 | 5 | 0.164055 | 0.156868 | 0.172994 | 0.163623 | 1.6363 | true | 5.5039 | final_report_sweep |
| 2025-12-17T08:47:56Z | unknown | proposed | hybrid | 1024 | 2 | 4 | 5 | 1.251985 | 1.206425 | 1.392611 | 1.275260 | 1.7153 | true | 9.3880 | final_report_sweep |
| 2025-12-17T08:51:23Z | unknown | proposed | hybrid | 2048 | 2 | 4 | 5 | 10.305494 | 9.880740 | 10.843042 | 10.383362 | 1.6671 | true | 13.0258 | final_report_sweep |
| 2025-12-17T08:51:24Z | unknown | naive | hybrid | 128 | 4 | 2 | 5 | 0.002732 | 0.002151 | 0.003471 | 0.002656 | 1.5350 | true | 2.3897 | final_report_sweep |
| 2025-12-17T08:51:24Z | unknown | naive | hybrid | 256 | 4 | 2 | 5 | 0.021641 | 0.020584 | 0.022554 | 0.021557 | 1.5505 | true | 3.6445 | final_report_sweep |
| 2025-12-17T08:51:27Z | unknown | naive | hybrid | 512 | 4 | 2 | 5 | 0.306588 | 0.298044 | 0.315511 | 0.305270 | 0.8756 | true | 3.0253 | final_report_sweep |
| 2025-12-17T08:52:11Z | unknown | naive | hybrid | 1024 | 4 | 2 | 5 | 4.354803 | 4.197581 | 4.474276 | 4.357948 | 0.4931 | true | 3.0391 | final_report_sweep |
| 2025-12-17T08:59:38Z | unknown | naive | hybrid | 2048 | 4 | 2 | 5 | 43.699052 | 43.188327 | 45.888383 | 44.061795 | 0.3931 | true | 3.1733 | final_report_sweep |
| 2025-12-17T08:59:39Z | unknown | strassen | hybrid | 128 | 4 | 2 | 5 | 0.005905 | 0.005829 | 0.006621 | 0.006153 | 0.7102 | true | 1.4152 | final_report_sweep |
| 2025-12-17T08:59:40Z | unknown | strassen | hybrid | 256 | 4 | 2 | 5 | 0.034268 | 0.032712 | 0.041460 | 0.036022 | 0.9792 | true | 2.3234 | final_report_sweep |
| 2025-12-17T08:59:43Z | unknown | strassen | hybrid | 512 | 4 | 2 | 5 | 0.257298 | 0.233502 | 0.320921 | 0.264391 | 1.0433 | true | 3.9011 | final_report_sweep |
| 2025-12-17T09:00:04Z | unknown | strassen | hybrid | 1024 | 4 | 2 | 5 | 1.466324 | 1.423573 | 1.482512 | 1.461252 | 1.4645 | true | 7.7033 | final_report_sweep |
| 2025-12-17T09:03:27Z | unknown | strassen | hybrid | 2048 | 4 | 2 | 5 | 10.674763 | 10.491154 | 10.736530 | 10.618632 | 1.6094 | true | 11.8652 | final_report_sweep |
| 2025-12-17T09:03:28Z | unknown | proposed | hybrid | 128 | 4 | 2 | 5 | 0.002159 | 0.001781 | 0.002333 | 0.002122 | 1.9429 | true | 3.2342 | final_report_sweep |
| 2025-12-17T09:03:28Z | unknown | proposed | hybrid | 256 | 4 | 2 | 5 | 0.012549 | 0.011560 | 0.013466 | 0.012708 | 2.6739 | true | 6.0116 | final_report_sweep |
| 2025-12-17T09:03:30Z | unknown | proposed | hybrid | 512 | 4 | 2 | 5 | 0.096047 | 0.088936 | 0.108332 | 0.096714 | 2.7948 | true | 9.3332 | final_report_sweep |
| 2025-12-17T09:03:46Z | unknown | proposed | hybrid | 1024 | 4 | 2 | 5 | 0.658246 | 0.645965 | 0.665685 | 0.657642 | 3.2624 | true | 17.4219 | final_report_sweep |
| 2025-12-17T09:06:36Z | unknown | proposed | hybrid | 2048 | 4 | 2 | 5 | 5.198137 | 5.158606 | 5.544054 | 5.319142 | 3.3050 | true | 25.3534 | final_report_sweep |
| 2025-12-17T09:06:37Z | unknown | naive | hybrid | 128 | 4 | 1 | 5 | 0.002081 | 0.002019 | 0.002542 | 0.002180 | 2.0155 | true | 2.8385 | final_report_sweep |
| 2025-12-17T09:06:37Z | unknown | naive | hybrid | 256 | 4 | 1 | 5 | 0.021000 | 0.019839 | 0.022452 | 0.021028 | 1.5978 | true | 3.5332 | final_report_sweep |
| 2025-12-17T09:06:40Z | unknown | naive | hybrid | 512 | 4 | 1 | 5 | 0.285127 | 0.270818 | 0.327206 | 0.289549 | 0.9415 | true | 3.1892 | final_report_sweep |
| 2025-12-17T09:07:16Z | unknown | naive | hybrid | 1024 | 4 | 1 | 5 | 3.547552 | 3.491471 | 3.602788 | 3.542906 | 0.6053 | true | 3.0640 | final_report_sweep |
| 2025-12-17T09:14:34Z | unknown | naive | hybrid | 2048 | 4 | 1 | 5 | 43.250924 | 42.506506 | 43.830064 | 43.251457 | 0.3972 | true | 3.0897 | final_report_sweep |
| 2025-12-17T09:14:34Z | unknown | strassen | hybrid | 128 | 4 | 1 | 5 | 0.006198 | 0.005944 | 0.006446 | 0.006185 | 0.6767 | true | 1.0841 | final_report_sweep |
| 2025-12-17T09:14:35Z | unknown | strassen | hybrid | 256 | 4 | 1 | 5 | 0.033367 | 0.030243 | 0.034337 | 0.032597 | 1.0056 | true | 2.2211 | final_report_sweep |
| 2025-12-17T09:14:37Z | unknown | strassen | hybrid | 512 | 4 | 1 | 5 | 0.220744 | 0.211618 | 0.229193 | 0.219746 | 1.2160 | true | 3.9794 | final_report_sweep |
| 2025-12-17T09:14:59Z | unknown | strassen | hybrid | 1024 | 4 | 1 | 5 | 1.419936 | 1.406496 | 1.446579 | 1.424641 | 1.5124 | true | 8.0764 | final_report_sweep |
| 2025-12-17T09:18:21Z | unknown | strassen | hybrid | 2048 | 4 | 1 | 5 | 10.603364 | 10.393040 | 10.681647 | 10.569201 | 1.6202 | true | 11.9116 | final_report_sweep |
| 2025-12-17T09:18:22Z | unknown | proposed | hybrid | 128 | 4 | 1 | 5 | 0.001966 | 0.001886 | 0.002381 | 0.002060 | 2.1339 | true | 3.4726 | final_report_sweep |
| 2025-12-17T09:18:22Z | unknown | proposed | hybrid | 256 | 4 | 1 | 5 | 0.011674 | 0.010303 | 0.012505 | 0.011596 | 2.8742 | true | 6.3756 | final_report_sweep |
| 2025-12-17T09:18:24Z | unknown | proposed | hybrid | 512 | 4 | 1 | 5 | 0.102296 | 0.093054 | 0.153749 | 0.110416 | 2.6241 | true | 8.9654 | final_report_sweep |
| 2025-12-17T09:18:39Z | unknown | proposed | hybrid | 1024 | 4 | 1 | 5 | 0.646716 | 0.626583 | 0.691908 | 0.654269 | 3.3206 | true | 16.3962 | final_report_sweep |
| 2025-12-17T09:21:31Z | unknown | proposed | hybrid | 2048 | 4 | 1 | 5 | 5.175994 | 5.063242 | 5.590396 | 5.284855 | 3.3191 | true | 26.0137 | final_report_sweep |
