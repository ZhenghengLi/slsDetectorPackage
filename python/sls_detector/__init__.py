from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .experimental import ExperimentalDetector
from .jungfrau import Jungfrau
from .jungfrau_ctb import JungfrauCTB
from _sls_detector import DetectorApi

import _sls_detector

defs = _sls_detector.slsDetectorDefs
runStatus = _sls_detector.slsDetectorDefs.runStatus
detectorType = _sls_detector.slsDetectorDefs.detectorType
detectorSettings = _sls_detector.slsDetectorDefs.detectorSettings
