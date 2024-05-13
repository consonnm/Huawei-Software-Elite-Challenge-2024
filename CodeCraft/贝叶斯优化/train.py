import os
from skopt import forest_minimize
from skopt.space import Real
from skopt.space import Categorical
import glob
from log_util import get_log
from multiprocessing import Pool

log, file_path = get_log("train")

total_run = 1000


def train(path, params):
    judge = "./PreliminaryJudge"
    exe = "./main"
    cmd = f'{judge} -m {path} "{exe} {params}" -l ERR -s 9895'
    result = os.popen(cmd).read()
    output = dict(eval(result))
    # {"status":"Successful","score":89424}
    if output['status'] == 'Successful':
        score = output['score']
    else:
        score = -1
    # print(f"Map: {path}, Score: {score}")
    return score


def run_program(params):
    params = " ".join(map(str, params))

    paths = sorted(glob.glob("./maps_all/*.txt"))
    # print(train(paths[0], params))

    with Pool() as p:
        score_list = p.starmap(train, [(path, params) for path in paths])
    score_sum = sum(score_list)
    avg_score = score_sum / len(score_list)
    log.info(f"params: {params}")
    log.info(f"Score list: {score_list}")
    log.info(f"Total score: {score_sum}")
    log.info(f"Average score: {avg_score}")
    return -avg_score


def print_status(res):
    log.info(f"After call {len(res.func_vals)}: Best score: {-res.fun}")
    log.info(f"With parameters: {res.x}")
    log.info("====================================\n")


if __name__ == "__main__":
    param_space = [
        Real(0, 1.0, name='w_good_val'),
        Real(0, 1.0, name='w_good_dis'),
        Real(0, 1.0, name='w_boat_speed'),
        Real(0, 2.0, name='w_boat_size'),
        Real(0, 1.0, name='w_boat_transport'),
        Real(0, 2.0, name='w_berth_fill'),
        Real(0, 1.0, name='w_berth_dis'),
        Real(0, 2.0, name='w_berth_vis'),
        Real(0, 1.0, name='w_near_dis'),
        Real(0, 1.0, name='w_near_val'),
    ]

    # 使用贝叶斯优化
    result = forest_minimize(run_program, param_space, n_calls=total_run, callback=[print_status], random_state=0,
                             verbose=True)

    # 输出最优参数及其得分
    log.info("最优参数：", result.x)
    log.info("最佳得分：", -result.fun)
