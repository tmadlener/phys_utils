def setup_logger(**kwargs):
    """Small helper funciton for basic logger setup"""
    import logging
    import sys

    # get the name from the kwargs before passing them on to basicConfig
    name = kwargs.pop('name', __name__)
    level = kwargs.pop('level', 'INFO').upper()

    if level not in ['ERROR', 'INFO', 'WARNING', 'DEBUG', 'CRITICAL']:
        print('Invalid logging level passed: {}'.format(level))
        sys.exit(1)

    # define default format
    if 'format' not in kwargs:
        kwargs['format'] = '%(levelname)s - %(funcName)s: %(message)s'

        logging.basicConfig(**kwargs)

    logger = logging.getLogger(name)
    logger.setLevel(getattr(logging, level))

    return logger
