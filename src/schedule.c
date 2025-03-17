#include "schedule.h"

const struct hhmm schedule_monday[BELLS_PER_DAY] = {
      { .hour = 8 , .minute = 0  }, { .hour = 8 , .minute = 35 }		// 1 смена, 1 урок
    , { .hour = 8 , .minute = 45 }, { .hour = 9 , .minute = 20 }		// 1 смена, 2 урок
    , { .hour = 9 , .minute = 30 }, { .hour = 10, .minute = 5  }		// 1 смена, 3 урок
    , { .hour = 10, .minute = 20 }, { .hour = 10, .minute = 55 }		// 1 смена, 4 урок
    , { .hour = 11, .minute = 10 }, { .hour = 11, .minute = 45 }		// 1 смена, 5 урок
    , { .hour = 11, .minute = 55 }, { .hour = 12, .minute = 30 }		// 1 смена, 6 урок
    , { .hour = 12, .minute = 35 }, { .hour = 13, .minute = 10 }		// 1 смена, 7 урок

    , { .hour = 13, .minute = 15 }, { .hour = 13, .minute = 50 }		// 2 смена, 1 урок
    , { .hour = 14, .minute = 0  }, { .hour = 14, .minute = 35 }		// 2 смена, 2 урок
    , { .hour = 14, .minute = 45 }, { .hour = 15, .minute = 20 }		// 2 смена, 3 урок
    , { .hour = 15, .minute = 25 }, { .hour = 16, .minute = 0  }		// 2 смена, 4 урок
    , { .hour = 16, .minute = 5  }, { .hour = 16, .minute = 40 }		// 2 смена, 5 урок
    , { .hour = 16, .minute = 45 }, { .hour = 17, .minute = 20 }		// 2 смена, 6 урок
};
const struct hhmm schedule_other_day[BELLS_PER_DAY] = {
      { .hour = 8 , .minute = 0  }, { .hour = 8 , .minute = 40 }		// 1 смена, 1 урок
    , { .hour = 8 , .minute = 50 }, { .hour = 9 , .minute = 30 }		// 1 смена, 2 урок
    , { .hour = 9 , .minute = 40 }, { .hour = 10, .minute = 20 }		// 1 смена, 3 урок
    , { .hour = 10, .minute = 35 }, { .hour = 11, .minute = 15 }		// 1 смена, 4 урок
    , { .hour = 11, .minute = 30 }, { .hour = 12, .minute = 10 }		// 1 смена, 5 урок
    , { .hour = 12, .minute = 15 }, { .hour = 12, .minute = 55 }		// 1 смена, 6 урок
    , { .hour = 13, .minute = 0  }, { .hour = 13, .minute = 40 }		// 1 смена, 7 урок

    , { .hour = 14, .minute = 0  }, { .hour = 14, .minute = 40 }		// 2 смена, 1 урок
    , { .hour = 14, .minute = 50 }, { .hour = 15, .minute = 30 }		// 2 смена, 2 урок
    , { .hour = 15, .minute = 40 }, { .hour = 16, .minute = 20 }		// 2 смена, 3 урок
    , { .hour = 16, .minute = 25 }, { .hour = 17, .minute = 5  }		// 2 смена, 4 урок
    , { .hour = 17, .minute = 10 }, { .hour = 17, .minute = 50 }		// 2 смена, 5 урок
    , { .hour = 17, .minute = 55 }, { .hour = 18, .minute = 35 }		// 2 смена, 6 урок
};
