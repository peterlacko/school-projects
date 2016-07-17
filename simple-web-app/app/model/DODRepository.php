<?php

namespace Poradna;
use Nette;

/**
 *	DOD repository 
 */
class DodRepository extends Repository
{
    public function showDODs()
    {
		return $this->findAll()->order('Zacatek', 'ASC');
    }

    public function deleteDod($id)
    {
		$this->findBy(array('ID_DOD'=>$id))->delete();
    }

    public function addDod($values, $id_fakulty)
    {
	$this->getTable()->insert(array(
	    'Zacatek' => $values->zacatek,
	    'Konec' => $values->konec,
	    'Podrobnosti' => $values->podrobnosti,
	    'ID_fakulty' => $id_fakulty
	));
    }

    public function search($phrase)
    {
		return $this->connection->table('dod')->where("Podrobnosti LIKE ?", "%$phrase%");
    }
}
