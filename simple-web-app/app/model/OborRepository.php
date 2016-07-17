<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka user
 *   */
class OborRepository extends Repository
{
    public function deleteObor($id)
    {
	$this->findBy(array('ID_oboru'=>$id))->delete();
    }

    public function addObor($values, $id_fakulty) 
    {
	$this->getTable()->insert(array(
	    'Nazev_oboru' => $values->nazev_oboru,
	    'Delka_studia' => $values->delka_studia,
	    'Forma_studia' => $values->forma_studia,
	    'Titul' => $values->titul,
	    'Zakonceni' => $values->zakonceni,
	    'Popis' => $values->popis,
	    'ID_fakulty' => $id_fakulty
	));
    }

    public function isOborInFakulta($nazev_oboru, $id_fakulty)
    {
	if ($this->getTable()->where(array(
	    'Nazev_oboru' => $nazev_oboru,
	    'ID_fakulty' => $id_fakulty))->fetch()) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    public function search($phrase)
    {
	return $this->connection->table('obor')->where("Nazev_oboru LIKE ?", "%$phrase%");
    }
}
