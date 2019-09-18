from hwd.dts.common import node, Peripheral, Number

@node
class Nvic(Peripheral):
    """Device Tree Binding for the Nested Vectored Interrupt Controller present
    in ARM-v7-m architectures.

    Properties:
        - interrupt_cells: Number of cells needed to define an interrupt.

    Note: This node is an interrupt controller.
    """
    _node_name = 'interrupt-controller'
    interrupt_cells = Number(column_name="interrupt_cells")

    class CellData:
        """Phandles that reference the NVIC.

        Cells:
            - line: Interrupt line
        """
        cells = ['line']
